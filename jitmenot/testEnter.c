#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jitmenot.h"
#include "testEnter.h"

static sigjmp_buf afterSIGILL;
volatile static sig_atomic_t catchedSignal = -1;

static void hdl(int sig) {
    catchedSignal = sig;
    siglongjmp(afterSIGILL, -1);
    return;
}

static int detect(void) {
    if (sigsetjmp(afterSIGILL, 1) != 0) {
#ifdef DEBUG
        printf("Received signal:\t%d\n", catchedSignal);
#endif
        return RESULT_YES;
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

    asm volatile("enter $0, $1\n\t"
                 "leave");

    return RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_enter(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_ENTER;

    test->detect = detect;
    test->description = TEST_DESC_ENTER;
    test->name = TEST_NAME_ENTER;
    test->cleanup = cleanup;

    return 0;
}
