#include "x86intrin.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "jitmenot.h"
#include "testJITLibTime.h"

const static char *linux_common_libs[NUM_LIBS] = {"libpthread.so.0", "libutil.so", "libcrypt.so", "libselinux.so.1",
                                                  "libpcre.so.3"};

int load_unload_libs(const char **libs, int length) {
    void *handle;
    for (int i = 0; i < length; i++) {
        if ((handle = dlopen(libs[i], RTLD_NOW)) == NULL) {
#ifdef DEBUG
            printf("Could not open %s\n", libs[i]);
#endif
            return -1;
        }

        if (handle != NULL && dlclose(handle) != 0) {
#ifdef DEBUG
            printf("Could not close %s\n", libs[i]);
#endif
            return -1;
        }
    }

    return 0;
}

static int detect(void) {
    unsigned long long start[NUM_LOADS], end[NUM_LOADS];

    for (int i = 0; i < NUM_LOADS; i++) {
        start[i] = _rdtsc();

        if (load_unload_libs(linux_common_libs, NUM_LIBS) < 0) {
            return RESULT_UNK;
        }

        end[i] = _rdtsc();
    }

    for (int i = 0; i < NUM_LOADS; i++) {
        if (end[i] < start[i]) {
#ifdef DEBUG
            printf("Start %llu and End %llu times not monotonically increasing\n", start[i], end[i]);
#endif
            return RESULT_UNK;
        }
    }

    long long first_load = end[0] - start[0];
    long long second_load = end[1] - start[1];
    double difference = (double)second_load / first_load;

#ifdef DEBUG
    printf("First lib load time:\t%lld\n", first_load);
    printf("Second lib load time:\t%lld\n", second_load);
    printf("Difference:\t%f\n", difference);
#endif

    return difference < 0.36 ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_jitlib(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_JITLIB;

    test->detect = detect;
    test->description = TEST_DESC_JITLIB;
    test->name = TEST_NAME_JITLIB;
    test->cleanup = cleanup;

    return 0;
}
