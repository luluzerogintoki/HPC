#pragma once

#include <inttypes.h>
#include <complex.h>

#define ATTR_PATH "newton_attractors_xd.ppm"
#define CONV_PATH "newton_convergence_xd.ppm"

// This is a cute way of defining the types, but the code in writing.c assumes that they are both uint8_t.
// If this changes, the code WILL crash.
#define TYPE_ATTR uint8_t
#define TYPE_CONV uint8_t

// Coordinate range
#define MIN_NUMBER -2
#define MAX_NUMBER 2

#define ITERS_CUTOFF 128 
#define EPSILON 1e-3f
#define EPSILON2 1e-6f // Epsilon squared

// infinite attractor
#define MAX_ABS_VALUE 1e10f
#define MIN_ABS_VALUE -1e10f

#define complex_t float _Complex
