#pragma once

#include <stdbool.h>
#include <threads.h>
#include "constants.h"

// Do not forget to define each of these extern lines at some point in your code, or it won't work.

extern int n_threads;
extern int n_lines;
extern int degree;

extern int32_t *curr_line;         // The current line of each thread. MUST BE INITIALIZED TO -1.
extern mtx_t *mtx_ptr;
extern cnd_t *cnd_ptr;
