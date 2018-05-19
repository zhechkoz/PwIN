#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "jitmenot.h"
#include "testSMC.h"

static int detect(void) {
    volatile int change_me = 0;

    int page_size = getpagesize();
    extern unsigned char mov_label[];
    unsigned char *page_start = mov_label - (unsigned long)mov_label % page_size;
    unsigned char *mov_instr_address = mov_label + 0x1;

#ifdef DEBUG
    const char *error_mprotect = "mprotect couldn't change the permissions of text segment.";
#endif

    if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
#ifdef DEBUG
        puts(error_mprotect);
#endif
        return RESULT_UNK;
    }

    *mov_instr_address = 0x0;

    // This will not work if Pin was executed with -smc_strict 1
    asm volatile(".global mov_label\n\t"
                 "mov_label: \n\t"
                 "mov $1, %%eax\n\t"
                 "mov %%eax, %0"
                 : "=r"(change_me)
                 :
                 : "rax");

    if (mprotect(page_start, page_size, PROT_READ | PROT_EXEC) == -1) {
#ifdef DEBUG
        puts(error_mprotect);
#endif
        return RESULT_UNK;
    }

#ifdef DEBUG
    printf("change_me\t%d\n", change_me);
#endif

    return change_me == 1 ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_smc(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_SMC;

    test->detect = detect;
    test->description = TEST_DESC_SMC;
    test->name = TEST_NAME_SMC;
    test->cleanup = cleanup;

    return 0;
}
