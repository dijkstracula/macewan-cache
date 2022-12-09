#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "utils.h"

/* Returns the number of seconds passed between the two timespecs. */
double diff_timespec(const struct timespec *time1, const struct timespec *time0) {
      return (time1->tv_sec - time0->tv_sec) + (time1->tv_nsec - time0->tv_nsec) / 1000000000.0;
}


/* Wraps calls to malloc(): if it fails, exit the program. */
void *xmalloc(size_t sz, const char *function, uint64_t line) {
    void *ret = calloc(sz, 1);
    if (!ret) {
        fprintf(stderr, "Allocation for %ld bytes failed in %s() on line %ld!\n", sz, function, line);
        exit(1);
    }
    return ret;
}

