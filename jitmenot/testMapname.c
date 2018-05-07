#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jitmenot.h"
#include "pmparser.h"
#include "testMapname.h"

static const char *pinbin = "pinbin";
static const char *dynamorio = "libdynamorio.so";
static const char *dyninstAPI = "libdyninstAPI_RT";
static const char *vgpreload = "vgpreload";

static int detect(void) {
    int detected = 0;
    procmaps_struct *maps = pmparser_parse(-1); // this process

    if (maps == NULL) {
#ifdef DEBUG
        printf("Cannot parse the memory map!\n");
#endif
        return RESULT_UNK;
    }

    procmaps_struct *maps_tmp = NULL;

    while ((maps_tmp = pmparser_next()) != NULL) {
        char *pathname = maps_tmp->pathname;
        if (strstr(pathname, pinbin) != NULL || strstr(pathname, vgpreload) != NULL ||
            strstr(pathname, dyninstAPI) != NULL || strstr(pathname, dynamorio) != NULL) {
            detected = 1;
            break;
        }
    }

    pmparser_free(maps);

    return detected == 1 ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_mapname(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_MAPNAME;

    test->detect = detect;
    test->description = TEST_DESC_MAPNAME;
    test->name = TEST_NAME_MAPNAME;
    test->cleanup = cleanup;

    return 0;
}
