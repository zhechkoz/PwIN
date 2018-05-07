#define _GNU_SOURCE
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/auxv.h>

#include "jitmenot.h"

#include "testEnter.h"
#include "testEnvVar.h"
#include "testFsbase.h"
#include "testJITBranchTime.h"
#include "testJITLibTime.h"
#include "testMapname.h"
#include "testNX.h"
#include "testPagePerm.h"
#include "testRIPFXSAVE.h"
#include "testRIPSIGINFO.h"
#include "testRIPSYSCALL.h"
#include "testSMC.h"
#include "testVMLeave.h"

unsigned int this_arch = UINT_MAX;

const char *arch_strings[] = {
        [ARCH_AMD64] = "x86_64",
};

struct test_chain *test_chain_alloc_new(struct test_chain *all_tests) {
    struct test_chain *test = NULL, *tail = NULL;

    if (!all_tests) {
#ifdef DEBUG
        fprintf(stderr, "BUG: all_tests is NULL!\n");
#endif
        return NULL;
    }

    test = calloc(1, sizeof(struct test_chain));
    if (!test) {
        fprintf(stderr, "Out of memory when registering test file __FILE__\n");
        return NULL;
    }

    tail = all_tests;
    while (tail) {
        if (tail->next_test == NULL) {
            tail->next_test = test;
            break;
        }
        tail = tail->next_test;
    }

    return test;
}

void print_usage(char *prog_name) {
    printf("Usage: %s \n\t-v\t\t Verbose output\n\t-h\t\t Prints this message\n", prog_name);
}

void test_chain_free_all(struct test_chain *all_tests) {
    struct test_chain *next = NULL;

    if (!all_tests) {
#ifdef DEBUG
        fprintf(stderr, "BUG: all_tests is NULL!\n");
#endif
        return;
    }

    while (all_tests) {
        next = all_tests->next_test;
        all_tests->cleanup();
        free(all_tests);
        all_tests = next;
    }
    return;
}

int main(int argc, char **argv) {
    int res = 0, verbose = 0, option = 0;
    struct test_chain head = {NULL, NULL, NULL, NULL, NULL};
    struct test_chain *cur = NULL;

    // Check current architecture
    for (int i = 0; i < sizeof(arch_strings) / sizeof(arch_strings[0]); i++) {
        if (!strcmp((const char *)getauxval(AT_PLATFORM), arch_strings[i])) this_arch = i;
    }

    if (this_arch == UINT_MAX) {
        fprintf(stderr, "Running on unsupported architecture.\n");
        return -1;
    }

    while ((option = getopt(argc, argv, "vh")) != -1) {
        switch (option) {
        case 'v':
            verbose = 1;
            break;
        case 'h':
        default:
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    head.name = "head";
    head.description = "List head. Should not be visible at all.";

    res |= register_test_jitbr(&head);
    res |= register_test_jitlib(&head);
    res |= register_test_pageperm(&head);
    res |= register_test_vmleave(&head);
    res |= register_test_mapname(&head);
    res |= register_test_smc(&head);
    res |= register_test_ripfxsave(&head);
    res |= register_test_testripsiginfo(&head);
    res |= register_test_ripsyscall(&head);
    res |= register_test_testnx(&head);
    res |= register_test_envvar(&head);
    res |= register_test_fsbase(&head);
    res |= register_test_enter(&head);

    if (res) {
        fprintf(stderr, "Failed to register one or more tests (bmp = %#018x)."
                        "Exiting ...",
                res);
        return -1;
    }

    for (cur = head.next_test; cur; cur = cur->next_test) {
        if (verbose != 0) {
            puts(cur->description);
        }

        printf("%12s: ", cur->name);
#ifdef DEBUG
        puts("");
#endif
        switch (cur->detect()) {
        case RESULT_NO:
            puts("\x1b[1m\x1b[32mNEGATIVE\x1b[0m\x1b[39m");
            break;
        case RESULT_YES:
            puts("\x1b[1m\x1b[31mPOSITIVE\x1b[0m\x1b[39m");
            break;
        case RESULT_UNK:
            puts("\x1b[1m\x1b[33mUNKNOWN\x1b[0m\x1b[39m");
            break;
        default:
            fprintf(stderr, "BUG: Test %s returned invalid result! Enable debug!\n", cur->name);
            return -1;
        }

        if (verbose != 0) {
            puts("");
        }
    }

    puts("");
    test_chain_free_all(head.next_test);

    // scanf("%c", argv[0]);
}
