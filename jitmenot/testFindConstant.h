#ifndef _TEST_FIND_CONSTANT_H
#define _TEST_FIND_CONSTANT_H

#include "jitmenot.h"

int register_test_find_constant(struct test_chain *);

#define TEST_ID_FIND_CONSTANT 13
#define TEST_NAME_FIND_CONSTANT "constant"
#define TEST_DESC_FIND_CONSTANT                                                                                        \
    "Checks whether a constant declared in the text segment can be found at least twice in the readable memory."

#endif
