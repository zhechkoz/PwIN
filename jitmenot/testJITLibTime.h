#ifndef _TEST_JITLIB_H
#define _TEST_JITLIB_H

#include "jitmenot.h"

int register_test_jitlib(struct test_chain *);

#define NUM_LIBS 5
#define NUM_LOADS 2

#define TEST_ID_JITLIB 2
#define TEST_NAME_JITLIB "jitlib"
#define TEST_DESC_JITLIB "Detects time overhead when libraries are loaded."

#endif
