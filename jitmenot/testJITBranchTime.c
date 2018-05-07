#include "x86intrin.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "jitmenot.h"
#include "testJITBranchTime.h"

static int detect(void) {
    unsigned long long times[NUM_TIMES];
    times[0] = _rdtsc();

    for (int i = 1; i < NUM_TIMES; i++) {
        times[i] = _rdtsc();
    }

    /*
        The first trace which PIN compiles includes both 0 and 1 times.
        Since the for loop jumps in the middle of a compiled trace, although
        the code is already in code cache, the traces are not equal, so it
        has to be jitted again (1-2). Lastly, there exist a trace with the
        same start address in the code cache but the static context has to be
        regenerated again before execution (2-3). So, only after the 4. rdtsc
        execution, PIN uses exclusively the instructions residing in code cache.
    */
    long long jit_time = fmax(fmax(times[1] - times[0], times[2] - times[1]), times[3] - times[2]);

    long long current, max = 0;
    for (int i = 3; i < NUM_TIMES - 1; i++) {
        current = times[i + 1] - times[i];
        max = fmax(max, current);
    }

#ifdef DEBUG
    printf("\nJIT time:\t\t%lld\n", jit_time);

    for (int i = 3; i < NUM_TIMES - 1; i++) {
        printf("Reused cache time:\t%lld\n", times[i + 1] - times[i]);
    }
#endif

    return jit_time > 15 * max ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_jitbr(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_JITBR;

    test->detect = detect;
    test->description = TEST_DESC_JITBR;
    test->name = TEST_NAME_JITBR;
    test->cleanup = cleanup;

    return 0;
}
