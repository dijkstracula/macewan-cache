#ifndef __UTILS_H_
#define __UTILS_H_

#include <time.h>

double diff_timespec(const struct timespec *time1, const struct timespec *time0);

/* The following function and preprocessor directives make error-checking
 * memory allocations a bit nicer.  Figuring out what this does would be
 * good practice in understanding C macros!
 */

/* Wraps calls to malloc(): if it fails, exit the program. */
void *xmalloc(size_t sz, const char *function, uint64_t line);

/* a macro that calls xmalloc, with the name of the function and line number
 * already inserted in the function call (recall that __FUNCTION__ and __LINE__
 * are built-in C macros - they get assigned values whereever they're used, so
 * they'll be assigned values whereever XMALLOC is expanded.)
 */
#define MALLOC(sz) xmalloc(sz, __FUNCTION__, __LINE__)

#endif
