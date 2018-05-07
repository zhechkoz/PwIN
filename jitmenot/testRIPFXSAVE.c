#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jitmenot.h"
#include "testRIPFXSAVE.h"

static int detect(void) {
    unsigned long rip = 0, real_rip = 0;
    unsigned long fxsave_memoty[FXSAVE_MEMORY_SIZE];
    extern unsigned char ripfxsave_label[];

    memset(fxsave_memoty, 0, sizeof(fxsave_memoty));

    // Execute a fp instruction and then save the
    // fp register values on the stack using fxsave64
    asm volatile(".global ripfxsave_label\n\t"
                 "leaq (%%rip), %0\n\t"
                 "ripfxsave_label: \n\t"
                 "fldpi\n\t"
                 "fxsave64 %1"
                 : "=r"(rip), "=m"(fxsave_memoty));

    real_rip = fxsave_memoty[1];

#ifdef DEBUG
    printf("\nRIP:\t\t0x%lx\nRealRIP:\t0x%lx\nRIPLabel:\t%p\n", rip, real_rip, ripfxsave_label);
#endif

    if (real_rip != (unsigned long)ripfxsave_label || real_rip != rip) {
        return RESULT_YES;
    }

    return RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_ripfxsave(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_RIPFXSAVE;

    test->detect = detect;
    test->description = TEST_DESC_RIPFXSAVE;
    test->name = TEST_NAME_RIPFXSAVE;
    test->cleanup = cleanup;

    return 0;
}
