#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jitmenot.h"
#include "testNX.h"

volatile static sig_atomic_t detected = RESULT_YES;
volatile static sig_atomic_t catched_signal = -1;
static sigjmp_buf after_sigsev;

/*
    call 0x5
    push rax
    mov eax, 0x2a
    pop rax
    ret
*/
static const unsigned char assembly[ASSEMBLY_SIZE] = {0xE8, 0x00, 0x00, 0x00, 0x00, 0x50, 0xB8,
                                                      0x2A, 0x00, 0x00, 0x00, 0x58, 0xC3};

static void hdl(int sig) {
    detected = RESULT_NO;

    catched_signal = sig;
    siglongjmp(after_sigsev, -1);
}

static int detect(void) {
    const int page_size = getpagesize();

    if (sigsetjmp(after_sigsev, 1) != 0) {
#ifdef DEBUG
        printf("Received signal:\t%d\n", catched_signal);
#endif
        return detected;
    }

    struct sigaction act;
    memset(&act, 0x0, sizeof(act));

    act.sa_handler = hdl;
    act.sa_flags = SA_RESETHAND;

    if (sigaction(SIGSEGV, &act, NULL)) {
#ifdef DEBUG
        printf("Sigaction unsuccessful - %s\n", strerror(errno));
#endif
        return RESULT_UNK;
    }

    unsigned char *to_exec = malloc(page_size);
    if (to_exec == NULL) {
#ifdef DEBUG
        printf("Malloc with size %d unsuccessful - %s\n", page_size, strerror(errno));
#endif
        return RESULT_UNK;
    }

    memcpy(to_exec, assembly, ASSEMBLY_SIZE);

    asm volatile("call *%0" : : "m"(to_exec));

    return detected;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_testnx(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_NX;

    test->detect = detect;
    test->description = TEST_DESC_NX;
    test->name = TEST_NAME_NX;
    test->cleanup = cleanup;

    return 0;
}
