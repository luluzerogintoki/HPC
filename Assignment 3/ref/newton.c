#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <math.h>

#include "constants.h"
#include "global.h"
#include "writing.h"

typedef struct {
    TYPE_ATTR **attractors;
    TYPE_CONV **convergences;
    int ib;
    int istep;
    int n_lines;
    int tx;
    float (*roots)[2];
    mtx_t *mtx_ptr;
    cnd_t *cnd_ptr;
    int *status;
} comp_thread_args_t;

typedef struct {
    TYPE_ATTR **attractors;
    TYPE_CONV **convergences;
    int n_lines;
    mtx_t *mtx_ptr;
    cnd_t *cnd_ptr;
    int *status;
} write_thread_args_t;


// COPY PASTE FRIENDLY, put this in each test file
int n_threads = -1;
int n_lines = -1;
int degree = -1;

static inline complex_t newton_iteration(complex_t x, int degree);
static inline complex_t get_z0(int x_index, int y_index, int n_lines);
inline static int parse_cmd_args(int argc, char *argv[]);
int comp_thread(void *args);


int main(int argc, char *argv[])        // ./bin/newton -l{n_lines} -t{n_threads} -{degree}
{
    if (parse_cmd_args(argc, argv))
        return -1;

    // Initialize the global memory
    TYPE_ATTR **attractors = malloc(sizeof(TYPE_ATTR*) * n_lines);
    TYPE_CONV **convergences = malloc(sizeof(TYPE_CONV*) * n_lines);
    int32_t *status = malloc(sizeof(int32_t) * n_threads);

    // Init synchronization tools
    cnd_t cnd;
    mtx_t mtx;
    cnd_t *cnd_ptr = &cnd;
    mtx_t *mtx_ptr = &mtx;
    cnd_init(cnd_ptr);
    mtx_init(mtx_ptr, mtx_plain);

    // Calculate exact roots to compare to
    float roots[degree][2];
    for (int i = 0; i < degree; ++i) {
        float mul = 2.f * 3.14159265358979323846f * ((float)i / (float)degree); // M_PI constant was sometimes giving an error
        roots[i][0] = cosf(mul);
        roots[i][1] = sinf(mul);
    }


    // Create threads
    thrd_t thrds[n_threads];
    comp_thread_args_t thread_args[n_threads];
    int szloc = n_lines / n_threads;
    for ( int tx = 0; tx < n_threads; ++tx ) {

        thread_args[tx].attractors = attractors;
        thread_args[tx].convergences = convergences;

        thread_args[tx].ib = tx;
        thread_args[tx].istep = n_threads;

        thread_args[tx].n_lines = n_lines;
        thread_args[tx].tx = tx;
        thread_args[tx].roots = roots;

        thread_args[tx].mtx_ptr = mtx_ptr;
        thread_args[tx].cnd_ptr = cnd_ptr;
        status[tx] = -1;
        thread_args[tx].status = status;


        int r = thrd_create(thrds+tx, comp_thread, (void*) (thread_args+tx));
        if ( r != thrd_success ) {
            fprintf(stderr, "failed to create thread\n");
            exit(1);
        }
        thrd_detach(thrds[tx]);
    }


    // TODO: Spawn writing thread - remove the other write-to-file as well
    thrd_t w_thread;
    {
        write_thread_args_t w_thread_args;
        w_thread_args.attractors = attractors;
        w_thread_args.convergences = convergences;

        w_thread_args.n_lines = n_lines;

        w_thread_args.mtx_ptr = mtx_ptr;
        w_thread_args.cnd_ptr = cnd_ptr;
        w_thread_args.status = status;

        int r = thrd_create(&w_thread, write_thread, (void*) (&w_thread_args));
        if ( r != thrd_success ) {
            fprintf(stderr, "failed to create thread\n");
            exit(1);
        }
    }
    {
        int r;
        thrd_join(w_thread, &r);
    }

    mtx_destroy(&mtx);
    cnd_destroy(&cnd);

    free(attractors);
    free(convergences);
    free(status);

    return 0;
}

static inline int parse_cmd_args(int argc, char *argv[]) {
    // Getting cmd-line arguments
    for (int ix = 0; ix < argc; ++ix) {
        char *curr_arg = argv[ix];
        if (curr_arg[0] == '-') {
            if (curr_arg[1] == 't') {
                // We add 2 to discard the first two chars, "-t" in this case
                n_threads = atoi(curr_arg + 2);
            } else if (curr_arg[1] == 'l') {
                n_lines = atoi(curr_arg + 2);
            }
        } else if (argc > 1 && ix == argc - 1) {
            degree = atoi(curr_arg);
        }
    }
    if (n_threads == -1 || n_lines == -1 || degree == -1) {
        printf("Not all cmd-line args set\n");
        return -1;
    }

    return 0;
}


int comp_thread(void *args) {
    // Get the args
    const comp_thread_args_t *thread_args = (comp_thread_args_t*) args;
    TYPE_ATTR **attractors = thread_args->attractors;
    TYPE_CONV **convergences = thread_args->convergences;
    int ib = thread_args->ib;
    int istep = thread_args->istep;
    int n_lines = thread_args->n_lines;
    int tx = thread_args->tx;
    float (*roots)[2] = thread_args->roots;
    mtx_t *mtx_ptr = thread_args->mtx_ptr;
    cnd_t *cnd_ptr = thread_args->cnd_ptr;
    int *status = thread_args->status;

    // Compute!
    for (int i = ib; i < n_lines; i += istep) {
        TYPE_ATTR *line_attractors = (TYPE_ATTR*) malloc(n_lines * sizeof(TYPE_ATTR));
        TYPE_CONV *line_convergences = (TYPE_CONV*) malloc(n_lines * sizeof(TYPE_CONV));
        for (int j = 0; j < n_lines; ++j) {
            complex_t z = get_z0(j, i, n_lines);

            int iter = 0;
            int attr = 0; // Default attractor is zero
            for ( ; iter < ITERS_CUTOFF; ++iter) {
                float a = crealf(z);
                float b = cimagf(z);

                if (a < MIN_ABS_VALUE || a > MAX_ABS_VALUE ||
                    b < MIN_ABS_VALUE || b > MAX_ABS_VALUE) {
                        attr = degree + 1; // Infinite attractor
                        break;
                } else if (a*a + b*b < EPSILON2) {
                    attr = 0; // Zero attractor
                    break;
                }
                for (int iroot = 0; iroot < degree; ++iroot) {
                    float re_diff = a-roots[iroot][0];
                    float im_diff = b-roots[iroot][1];

                    if ((re_diff)*(re_diff) + (im_diff)*(im_diff) < EPSILON2) {
                        attr = iroot + 1;
                        break;
                    }
                }
                if (attr != 0) {
                    break;
                }

                // Newton iteration
                z = newton_iteration(z, degree);
            }

            line_attractors[j] = attr;
            line_convergences[j] = iter;
        }
        mtx_lock(mtx_ptr);
        attractors[i] = line_attractors;
        convergences[i] = line_convergences;
        status[tx] = i + istep;
        mtx_unlock(mtx_ptr);
        cnd_signal(cnd_ptr);
    }

    return 0;
}


static inline complex_t newton_iteration(complex_t x, int degree)
{
  complex_t new_x;
  switch (degree){
    case 1:
      new_x = 1.f;
      break;
    case 2:
      new_x = 0.5f * x + 0.5f / x;
      break;

    case 3:
      new_x = 0.66666666666f * x + 0.3333333333f/(x * x);
      break;

    case 4:
      new_x = 0.75f * x + 0.25f / (x * x * x);
      break;
      
    case 5:
      new_x = 0.8f * x + 0.2f / (x * x * x * x);
      break;

    case 6:
      new_x = 0.8333333333f * x + 0.1666666666f / (x * x * x * x * x);
      break;

    case 7:
      new_x = 0.85714286f * x + 0.14285714f / (x * x * x * x * x * x);
      break;

    case 8:
      new_x = 0.875f * x + 0.125f / (x * x * x * x * x * x * x);
      break;

    case 9:
      new_x = 0.888888888f * x + 0.111111111f / (x * x * x * x * x * x * x * x);
      break;

    default:
      fprintf(stderr, "unexpected degree\n");
      exit(1);
  }
  return new_x;
}


static inline complex_t get_z0(int x_index, int y_index, int n_lines) {
  int len = MAX_NUMBER - MIN_NUMBER;
  float re = (float)len * (float)x_index/(float)(n_lines-1) + (float)MIN_NUMBER;
  float im = (float)len * (float)y_index/(float)(n_lines-1) + (float)MIN_NUMBER;

  return re - I*im;
}

