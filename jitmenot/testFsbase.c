#include "x86intrin.h"
#include <asm/prctl.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include "jitmenot.h"
#include "testFsbase.h"

static sigjmp_buf afterSIGILL;
volatile static sig_atomic_t catchedSignal = -1;
volatile static sig_atomic_t detected = RESULT_YES;

static void hdl(int sig) {
    detected = RESULT_UNK;

    catchedSignal = sig;
    siglongjmp(afterSIGILL, -1);
    return;
}

static int detect(void) {
    if (sigsetjmp(afterSIGILL, 1) != 0) {
#ifdef DEBUG
        printf("Received signal:\t%d\n", catchedSignal);
#endif
        return detected;
    }

    struct sigaction act;
    memset(&act, 0x0, sizeof(act));

    act.sa_handler = hdl;
    act.sa_flags = SA_RESETHAND;

    if (sigaction(SIGILL, &act, NULL)) {
#ifdef DEBUG
        printf("Sigaction unsuccessful - %s\n", strerror(errno));
#endif
        return RESULT_UNK;
    }

    unsigned long rdfsbase = _readfsbase_u64();

    unsigned long prctl_fsbase;
    if (syscall(SYS_arch_prctl, ARCH_GET_FS, &prctl_fsbase) == -1) {
#ifdef DEBUG
        printf("arch_prctl GET_FS failed - %s\n", strerror(errno));
#endif
        return RESULT_UNK;
    }

#ifdef DEBUG
    printf("rdfsbase:\t0x%lx\n", rdfsbase);
    printf("prctl:\t\t0x%lx\n", prctl_fsbase);
#endif

    return rdfsbase != prctl_fsbase ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_fsbase(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_FSBASE;

    test->detect = detect;
    test->description = TEST_DESC_FSBASE;
    test->name = TEST_NAME_FSBASE;
    test->cleanup = cleanup;

    return 0;
}
