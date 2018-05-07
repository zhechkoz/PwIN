#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jitmenot.h"
#include "testRIPSYSCALL.h"

static int detect(void) {
    unsigned long rip = 0, saved_rip = 0;
    extern unsigned char ripsys_label[];
    int page_size = getpagesize();

    // As described in https://software.intel.com/en-us/articles/intel-sdm on
    // sysenter the rip value is saved in rcx so in the end it is restored
    // by sysexit. When executed in Pin on sysexit the value is not rip.
    asm volatile(".global ripsys_label\n\t"
                 "mov $0, %%rcx\n\t"
                 "movq $0x27, %%rax\n\t"
                 "syscall\n\t"
                 "ripsys_label: \n\t"
                 "leaq (%%rip), %0\n\t"
                 "leaq (%%rcx), %1"
                 : "=r"(rip), "=r"(saved_rip)
                 :
                 : "rax", "rcx");

#ifdef DEBUG
    printf("\nRIP:\t\t0x%lx\nSavedRIP:\t0x%lx\nRIPLabel:\t%p\n", rip, saved_rip, ripsys_label);
#endif

    if (saved_rip != (unsigned long)ripsys_label || abs(saved_rip - rip) > page_size) {
        return RESULT_YES;
    }

    return RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_ripsyscall(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_RIPSYSCALL;

    test->detect = detect;
    test->description = TEST_DESC_RIPSYSCALL;
    test->name = TEST_NAME_RIPSYSCALL;
    test->cleanup = cleanup;

    return 0;
}
