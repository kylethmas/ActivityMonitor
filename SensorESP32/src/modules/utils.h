#ifndef UTILS_H
#define UTILS_H

/**
 * This module contains any small utility functions which might be useful to 
 * other modules.
 * 
 * @authors Thomas M.
*/

#include "config.h"
#include "debug.h"

#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))
#define CLAMP(x, mi, ma) (MIN(MAX(x, mi), ma))

#endif // UTILS_H