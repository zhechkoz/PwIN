#ifndef _TEST_JITBR_H
#define _TEST_JITBR_H

#include "jitmenot.h"

int register_test_jitbr(struct test_chain *);

#define NUM_TIMES 10

#define TEST_ID_JITBR 1
#define TEST_NAME_JITBR "jitbr"
#define TEST_DESC_JITBR "Detects time overhead when a conditional branch is jitted."

#endif
