/* itertest.c
 *
 * Benchmarks iterating through a linked list versus an array, to try to
 * observe the performance difference of the two O(n) operations owing to
 * spatial locality effects.
 *
 * author: taylorn5
 *
 * To compile and run:
 *   $ gcc -g -O0 -Wall -std=gnu99 itertest.c -o itertest
 *   $ ./itertest 20000000 array
 *   $ ./itertest 20000000 list
 *
 * To run under Cachegrind:
 *   $ valgrind --tool=cachegrind ./itertest 20000000 array
 *   $ valgrind --tool=cachegrind ./itertest 20000000 list
 * (Because Cachegrind has to simulate the behaviour of the cacheline
 * hierarchy, you should expect this to run quite a bit slower than normal.)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* How many distinct experiments to run (we'll average all their results */
#define ITERS 20

/* Returns the number of seconds passed between the two timespecs. */
double diff_timespec(const struct timespec *time1, const struct timespec *time0) {
      return (time1->tv_sec - time0->tv_sec) + (time1->tv_nsec - time0->tv_nsec) / 1000000000.0;
}

/* The following function and preprocessor directives make error-checking
 * memory allocations a bit nicer.  Figuring out what this does would be good
 * practice in understanding C macros!
 */

/* Wraps calls to malloc(): if it fails, exit the program. */
void *xmalloc(size_t sz, const char *function, uint64_t line) {
    void *ret = calloc(sz, 1);
    if (!ret) {
        fprintf(stderr, "Allocation for %ld bytes failed in %s() on line %ld!\n", sz, function, line);
        exit(1);
    }
    return ret;
}

/* a macro that calls xmalloc, with the name of the function and line number
 * already inserted in the function call (recall that __FUNCTION__ and __LINE__
 * are built-in C macros - they get assigned values whereever they're used,
 * so they'll be assigned values whereever XMALLOC is expanded.)
 */
#define XMALLOC(sz) xmalloc(sz, __FUNCTION__, __LINE__)


/* How big is our pool of struct nodes?  See doit_list for details. */
#define POOLSIZE 32

/* A simple linked list node. */
struct node {
    int *val;
    struct node *next;
};

/* Times how long it takes to initialise all elements of a linked list of size `n`. */
double doit_list(uint64_t size) {
    uint64_t i;
    struct timespec begin, end;
    struct node *curr = NULL;

    double results[ITERS];

    /* 0. Here we are doing something kind of funny: we preallocate a bunch of
     * nodes into a pool and, in the next step, build up the linked list from
     * randomly chosen elements in the pool.  Why are we doing this?
     *
     * If you think back to your memory allocator in CMPT 201, remember how the
     * first-fit allocation algorithm works: we'll walk the freelist until we
     * find a hole big enough to fit the requested storage object.  But, we're
     * just allocating the next node in a tight loop, without making other
     * allocations or freeing other allocations.  This means that, in the
     * first-fit algorithm, we'll actually have our linked list nodes be
     * adjacent to each other, so locality would be pretty good!
     *
     * In a real-world program, we would probably build up such a linked list
     * gradually while our program does other things, or splice a bunch of
     * different lists together, so this pool is meant to reflect that
     * property, namely that the order of the linked list isn't necessarily the
     * order that they were allocated in.
     *
     * For fun, make a prediction about how locality would change if you
     * modified the POOLSIZE constant on line 59, and then see if you're right!
     */
    struct node *node_pool[POOLSIZE];
    for (i = 0; i < POOLSIZE; i++) {
        node_pool[i] = XMALLOC(sizeof(struct node));
    }

    /* 1. Create a list of a given size... */
    struct node *head = NULL;
    for (i = 0; i < size; i++) {
        /* Choose a random allocated node from the pool. */
        int chosen = rand() % POOLSIZE;
        struct node *n = node_pool[chosen];

        /* Assign a value to the node's value. */
        n->val = XMALLOC(sizeof(int));
        n->next = head;
        head = n;

        /* Replace the node we took out of the pool with
         * a freshly-allocated copy. */
        node_pool[chosen] = XMALLOC(sizeof(struct node));
    }

    /* 2. OK, now do our benchmark... */
    for (i = 0; i < ITERS; i++) {
        fprintf(stderr, "\rTesting a list (%ld/%d)...", i+1, ITERS);
        curr = head;
        clock_gettime(CLOCK_MONOTONIC, &begin);
        while (curr != NULL) {
            *curr->val = i;
            curr = curr->next;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        results[i] = diff_timespec(&end, &begin);
    }

    /* 3. Don't forget to free our memory. */
    curr = head;
    while (head) {
        struct node *next = head->next;
        free(head->val);
        free(head);
        head = next;
    }
    for (i = 0; i < POOLSIZE; i++) {
        free(node_pool[i]);
    }

    /* 4. Return the average of all the trials. */
    double ret = 0.0;
    for (i = 0; i < ITERS; i++) {
        ret += results[i];
    }
    ret = ret/ITERS;
    return ret;
}


/* Times how long it takes to initialise all elements of an array of size `n` */
double doit_array(uint64_t size) {
    uint64_t i;
    struct timespec begin, end;
    double results[ITERS];

    /* 1. Create an array of a given size... */
    int *a = XMALLOC(size * sizeof(int));

    /* 2. OK, now do our benchmark... */
    for (int j = 0; j < ITERS; j++) {
        fprintf(stderr, "\rTesting an array (%d/%d)...", j+1, ITERS);
        clock_gettime(CLOCK_MONOTONIC, &begin);
        for (i = 0; i < size; i++) {
            a[i] = j;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        results[j] = diff_timespec(&end, &begin);
    }
    free(a);

    /* 3. Return the average of all the trials. */
    double ret = 0.0;
    for (i = 0; i < ITERS; i++) {
        ret += results[i];
    }
    ret = ret/ITERS;
    return ret;
}

void usage(char *progname) {
    fprintf(stderr, "Usage: %s <iteration count> <array|list>\n", progname);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        usage(argv[0]);
    }

    char *err = NULL;
    uint64_t iters = strtoll(argv[1], &err, 10);
    if (*err) {
        fprintf(stderr, "Invalid iteration count: expected a number, got %s\n", argv[1]);
        usage(argv[0]);
    }

    double ts = 0.0;
    if (strcmp(argv[2], "array") == 0) {
        ts = doit_array(iters);
    } else if (strcmp(argv[2], "list") == 0) {
        ts = doit_list(iters);
    } else {
        usage(argv[0]);
    }

    printf("Time to iterate through a %s of length %ld: %lf sec\n",
        argv[2], iters, ts);

    return 0;
}
