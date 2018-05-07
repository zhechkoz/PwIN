#ifndef _PIN_DETECT_H
#define _PIN_DETECT_H

#define VERSION "1.0"

#define RESULT_NO 0
#define RESULT_UNK 1
#define RESULT_YES 2

struct test_chain {
    /* Performs the actual test. Nonzero return means "debugger found" */
    int (*detect)(void);
    /* Cleanup any leftovers for asynchronous tests */
    int (*cleanup)(void);
    /* Human readable description of the test. */
    const char *description;
    /* Short, but UNIQUELY identifying name of the test. */
    const char *name;
    struct test_chain *next_test;
};

struct test_chain *test_chain_alloc_new(struct test_chain *);
void test_chain_free_all(struct test_chain *all_tests);

#if !defined __linux__
#error "Only supported operating system is Linux."
#endif

#if !defined __amd64__
#error "Compiling for unknown architecture. Only x86-64 is supported."
#endif

#define ARCH_AMD64 0

extern const char *arch_strings[];
extern unsigned int this_arch;

#endif
