/* This program demonstrates the effects of cacheline-level contention on a
 * multi-threaded program.  Each thread modifies an independent index in a
 * shared array; however, because multiple `int` values can fit on a single
 * cacheline, the hardware cache coherence protocol has to keep invalidating
 * the other CPU's caches, leading to performance problems.
 *
 * author: taylorn5
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <pthread.h>

#include "utils.h"

#define ITERS (1 << 29)

int num_cpus;

/* Here's our array of counters.  Remember that only thread i will
 * ever increment counters[i]. */
int *counters;

void print_counters() {
    for (int i = 0; i < num_cpus; i++) {
        printf("%8x ", counters[i]); 
    }
    printf("\n");
}

/* Each thread will run this function: it will increment its own private index
 * in the counters array some number of times and then return.  You can convince
 * yourself that it never accesses anyone else's counter value!
 */
void *thread_run(void *arg) {
    unsigned long thread_id = (unsigned long)(arg);
    printf("Hello from thread %ld\n", thread_id);
    for (int j = 1; j < ITERS; j++) {
        counters[thread_id]++;
    }
    printf("Farewell from thread %ld\n", thread_id);
    return NULL;
}

void usage(char *progname) {
    fprintf(stderr, "Usage: %s <cpus>\n", progname);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
    }

    char *err = NULL;
    num_cpus = strtoll(argv[1], &err, 10);
    if (*err) {
        fprintf(stderr, "Invalid CPU count: expected a number, got %s\n", argv[1]);
        usage(argv[0]);
    }

    /* 0. Init our array of counters. */
    counters = MALLOC(num_cpus * sizeof(int));
    print_counters();

    /* 1. Kick off a bunch of threads that will only ever
     * access their own counter index... */
    struct timespec begin, end;
    pthread_t *threads = MALLOC(num_cpus * sizeof(pthread_t));

    clock_gettime(CLOCK_MONOTONIC, &begin);
    for (unsigned long i = 0; i < num_cpus; i++) {
        int err;
        if ((err = pthread_create(&threads[i], NULL, thread_run, (void *)(i))) != 0) {
            fprintf(stderr, "Can't create a thread\n");
            exit(1);
        }
    }

    /* 2. Wait for all those threads to complete... */
    for (unsigned long i = 0; i < num_cpus; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    /* 3. Confirm that our counters are the values we expected! */
    print_counters();

    double ts = diff_timespec(&end, &begin);
    printf("Took %lf seconds for %d threads to complete\n", ts, num_cpus);
    free(counters);
    free(threads);
    return 0;
}
