#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* See feature_test_macros(7) */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jitmenot.h"
#include "pmparser.h"
#include "testFindConstant.h"

static int detect(void) {
    int detected = 0;
    void *found = NULL;
    unsigned long constant = 0xBBDFE5357130AF08;
    size_t constant_size = sizeof(constant);
    unsigned char *needle = malloc(constant_size);

    for (size_t i = 0; i < constant_size; i++) {
        needle[i] = *((unsigned char *)&constant + i);
    }

    procmaps_struct *maps = pmparser_parse(-1); // this process
    if (maps == NULL) {
#ifdef DEBUG
        printf("Cannot parse the memory map!\n");
#endif
        return RESULT_UNK;
    }

    procmaps_struct *maps_tmp = NULL;
    while ((maps_tmp = pmparser_next()) != NULL) {
        if (maps_tmp->is_r && strstr(maps_tmp->pathname, "vvar") == NULL) {
            if ((found = memmem(maps_tmp->addr_start, maps_tmp->length, needle, constant_size)) != NULL) {
#ifdef DEBUG
                printf("%p\n", found);
#endif
                detected += 1;
            }
        }
    }

    pmparser_free(maps);
    free(needle);
#ifdef DEBUG
    printf("%#lx was found %d time(s) in memory\n", constant, detected);
#endif

    // Normally *constant* can be found in .text, stack and heap
    return detected > 3 ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_find_constant(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_FIND_CONSTANT;

    test->detect = detect;
    test->description = TEST_DESC_FIND_CONSTANT;
    test->name = TEST_NAME_FIND_CONSTANT;
    test->cleanup = cleanup;

    return 0;
}
