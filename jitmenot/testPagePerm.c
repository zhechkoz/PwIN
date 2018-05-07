#include <stdio.h>
#include <stdlib.h>

#include "jitmenot.h"
#include "pmparser.h"
#include "testPagePerm.h"

static int detect(void) {
    int rwx = 0;
    procmaps_struct *maps = pmparser_parse(-1); // this process
    if (maps == NULL) {
#ifdef DEBUG
        printf("Cannot parse the memory map!\n");
#endif
        return RESULT_UNK;
    }

    procmaps_struct *maps_tmp = NULL;

    while ((maps_tmp = pmparser_next()) != NULL) {
        rwx += (maps_tmp->is_r && maps_tmp->is_w && maps_tmp->is_x) ? 1 : 0;
    }

    pmparser_free(maps);

    return rwx > 2 ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_pageperm(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_PAGEPERM;

    test->detect = detect;
    test->description = TEST_DESC_PAGEPERM;
    test->name = TEST_NAME_PAGEPERM;
    test->cleanup = cleanup;

    return 0;
}
