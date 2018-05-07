#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jitmenot.h"
#include "testRIPSIGINFO.h"

volatile unsigned long real_rip = 0;

static void hdl(int sig, siginfo_t *siginfo, void *context) {
    // Drill down the context using uc_mcontext struct and find
    // saved fpregs rip value
    ucontext_t *uc = (ucontext_t *)context;
    real_rip = (unsigned long)uc->uc_mcontext.fpregs->rip;
}

static int detect(void) {
    unsigned long rip = 0;
    extern unsigned char ripsiginfo_label[];

    struct sigaction act;
    memset(&act, 0x0, sizeof(act));

    act.sa_sigaction = hdl;
    act.sa_flags = SA_SIGINFO | SA_RESETHAND;

    if (sigaction(SIGTRAP, &act, NULL)) {
#ifdef DEBUG
        printf("Sigaction unsuccessful - %s\n", strerror(errno));
#endif
        return RESULT_UNK;
    }

    // Execute a harmless fp instruction and then an interrupt
    // which is mangaed in the registered handle
    asm volatile(".global ripsiginfo_label\n\t"
                 "leaq (%%rip), %0\n\t"
                 "ripsiginfo_label: \n\t"
                 "fldpi\n\t"
                 "int3"
                 : "=r"(rip));

#ifdef DEBUG
    printf("\nRIP:\t\t0x%lx\nRealRIP:\t0x%lx\nRIPLabel:\t%p\n", rip, real_rip, ripsiginfo_label);
#endif

    if (real_rip != (unsigned long)ripsiginfo_label || real_rip != rip) {
        return RESULT_YES;
    }

    return RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_testripsiginfo(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_RIPSIGINFO;

    test->detect = detect;
    test->description = TEST_DESC_RIPSIGINFO;
    test->name = TEST_NAME_RIPSIGINFO;
    test->cleanup = cleanup;

    return 0;
}
