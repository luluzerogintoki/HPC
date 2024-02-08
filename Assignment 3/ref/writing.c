#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "constants.h"
#include "global.h"
#include "writing.h"

# define COLOR_STR_SIZE 12
# define COLOR_MAX 255

#define PPM_HEADER "P3\n%10d %10d\n%10d\n"
#define PPM_HEADER_LEN 36


typedef struct {
    TYPE_ATTR **attractors;
    TYPE_CONV **convergences;
    int n_lines;
    mtx_t *mtx_ptr;
    cnd_t *cnd_ptr;
    int *status;
} write_thread_args_t;


const uint8_t ATTR_COLOR_VALUES[12][COLOR_STR_SIZE+1] = {
    "255 000 000 ",
    "255 000 127 ",
    "255 000 255 ",
    "127 000 255 ",
    "000 000 255 ",
    "000 127 255 ",
    "000 255 255 ",
    "000 255 127 ",
    "000 255 000 ",
    "127 255 000 ",
    "255 255 000 ",
    "255 127 000 ",
};

const uint8_t CONV_COLOR_VALUES[129][COLOR_STR_SIZE+1] = {
"000 000 000 ",
"002 002 002 ",
"004 004 004 ",
"006 006 006 ",
"008 008 008 ",
"010 010 010 ",
"012 012 012 ",
"014 014 014 ",
"016 016 016 ",
"018 018 018 ",
"020 020 020 ",
"022 022 022 ",
"024 024 024 ",
"026 026 026 ",
"028 028 028 ",
"030 030 030 ",
"032 032 032 ",
"034 034 034 ",
"036 036 036 ",
"038 038 038 ",
"040 040 040 ",
"042 042 042 ",
"044 044 044 ",
"046 046 046 ",
"048 048 048 ",
"050 050 050 ",
"052 052 052 ",
"054 054 054 ",
"056 056 056 ",
"058 058 058 ",
"060 060 060 ",
"062 062 062 ",
"064 064 064 ",
"066 066 066 ",
"068 068 068 ",
"070 070 070 ",
"072 072 072 ",
"074 074 074 ",
"076 076 076 ",
"078 078 078 ",
"080 080 080 ",
"082 082 082 ",
"084 084 084 ",
"086 086 086 ",
"088 088 088 ",
"090 090 090 ",
"092 092 092 ",
"094 094 094 ",
"096 096 096 ",
"098 098 098 ",
"100 100 100 ",
"102 102 102 ",
"104 104 104 ",
"106 106 106 ",
"108 108 108 ",
"110 110 110 ",
"112 112 112 ",
"114 114 114 ",
"116 116 116 ",
"118 118 118 ",
"120 120 120 ",
"122 122 122 ",
"124 124 124 ",
"126 126 126 ",
"128 128 128 ",
"130 130 130 ",
"132 132 132 ",
"134 134 134 ",
"136 136 136 ",
"138 138 138 ",
"140 140 140 ",
"142 142 142 ",
"144 144 144 ",
"146 146 146 ",
"148 148 148 ",
"150 150 150 ",
"152 152 152 ",
"154 154 154 ",
"156 156 156 ",
"158 158 158 ",
"160 160 160 ",
"162 162 162 ",
"164 164 164 ",
"166 166 166 ",
"168 168 168 ",
"170 170 170 ",
"172 172 172 ",
"174 174 174 ",
"176 176 176 ",
"178 178 178 ",
"180 180 180 ",
"182 182 182 ",
"184 184 184 ",
"186 186 186 ",
"188 188 188 ",
"190 190 190 ",
"192 192 192 ",
"194 194 194 ",
"196 196 196 ",
"198 198 198 ",
"200 200 200 ",
"202 202 202 ",
"204 204 204 ",
"206 206 206 ",
"208 208 208 ",
"210 210 210 ",
"212 212 212 ",
"214 214 214 ",
"216 216 216 ",
"218 218 218 ",
"220 220 220 ",
"222 222 222 ",
"224 224 224 ",
"226 226 226 ",
"228 228 228 ",
"230 230 230 ",
"232 232 232 ",
"234 234 234 ",
"236 236 236 ",
"238 238 238 ",
"240 240 240 ",
"242 242 242 ",
"244 244 244 ",
"246 246 246 ",
"248 248 248 ",
"250 250 250 ",
"252 252 252 ",
"254 254 254 ",
"255 255 255 ",
};


static inline void write_chunk(uint8_t **arr, uint32_t i_line_start, uint32_t i_line_end, int n_lines, FILE *stream, const uint8_t (*color_values)[COLOR_STR_SIZE+1]);
static inline void write_line_attr(uint8_t **arr, uint32_t i_line, int n_lines, FILE *stream);
static inline void write_line_conv(uint8_t **arr, uint32_t i_line, int n_lines, FILE *stream);


int write_thread(void *args) {
    const write_thread_args_t *thread_args = (write_thread_args_t*) args;
    TYPE_ATTR **attractors = thread_args->attractors;
    TYPE_CONV **convergences = thread_args->convergences;
    int n_lines = thread_args->n_lines;
    mtx_t *mtx_ptr = thread_args->mtx_ptr;
    cnd_t *cnd_ptr = thread_args->cnd_ptr;
    int *status = thread_args->status;

    char attr_path[26];
    char conv_path[27];
    sprintf(attr_path, "newton_attractors_x%d.ppm", degree);
    sprintf(conv_path, "newton_convergence_x%d.ppm", degree);
    FILE *attr_stream = fopen(attr_path, "wb");
    FILE *conv_stream = fopen(conv_path, "wb");
    fprintf(attr_stream, PPM_HEADER, n_lines, n_lines, COLOR_MAX);
    fprintf(conv_stream, PPM_HEADER, n_lines, n_lines, COLOR_MAX);

    for (int32_t i_write = 0, min_lines_done; i_write < n_lines;) {
        /* 
        * `i_write` is the index of the current line of the writing thread.
        * The loop will run until this variable catches up to the `min_lines_done` variable,
        * and then stall.
        *
        * `min_lines_done` contains the smallest number of lines definitely computed in the other threads.
        * It is the smallest value of `status` of each thread.
        *
        * The assumption is that each thread t writes on line `i_thread` + `i_line` * `num_threads`,
        * Under this assumption, the smallest `status` is indeed the smallest number of lines computed,
        * because the smallest line index implies that all the other lines are by definition past this one.
        * Since the `status` is initially set to -1, the loop will wait until each thread has completed at least one computation.
        */
        
        mtx_lock(mtx_ptr);
        while (true) {  // The code will only get past this block when the number of computed lines exceeds the current line.
            min_lines_done = n_lines;
            for (uint8_t i_thread = 0; i_thread < n_threads; ++i_thread)
                min_lines_done = (status[i_thread] < min_lines_done) ? status[i_thread] : min_lines_done;
            if (i_write < min_lines_done) {
                mtx_unlock(mtx_ptr);
                break;
            }

            // Relies on spurious wake up calls for the final iterations when the compute threads no longer signal. 
            // We can use cnd_timedwait instead, but this increases overhead.
            cnd_wait(cnd_ptr, mtx_ptr); 
        }

        write_chunk(attractors, i_write, min_lines_done, n_lines, attr_stream, ATTR_COLOR_VALUES);
        write_chunk(convergences, i_write, min_lines_done, n_lines, conv_stream, CONV_COLOR_VALUES);      

        for (; i_write < min_lines_done; ++i_write)  {
            free(attractors[i_write]);
            free(convergences[i_write]);
        }
    }

    fclose(attr_stream);
    fclose(conv_stream);
}


static inline
void
write_chunk(
        uint8_t **arr,
        uint32_t i_line_start,
        uint32_t i_line_end,
        int n_lines,
        FILE *stream,
        const uint8_t (*color_values)[COLOR_STR_SIZE+1]
    ) {
    const uint32_t chunk_size = i_line_end - i_line_start;
    char *packed_str = (char*) malloc(COLOR_STR_SIZE * chunk_size * n_lines * sizeof(char));
    for (uint32_t i_line = i_line_start, offset = 0; i_line < i_line_end; ++i_line) {
        for (uint32_t i = 0; i < n_lines; ++i, offset += COLOR_STR_SIZE) {
            const uint8_t val = arr[i_line][i];
            memcpy(packed_str + offset, color_values[val], COLOR_STR_SIZE);
        }
    }

    fseek(stream, PPM_HEADER_LEN + (i_line_start * n_lines * COLOR_STR_SIZE), SEEK_SET);
    fwrite(packed_str, sizeof(char), chunk_size * n_lines * COLOR_STR_SIZE, stream);
    fflush(stream);

    free(packed_str);
}
