#ifndef _TEST_FSBASE_H
#define _TEST_FSBASE_H

#include "jitmenot.h"

int register_test_fsbase(struct test_chain *);

#define TEST_ID_FSBASE 11
#define TEST_NAME_FSBASE "fsbase"
#define TEST_DESC_FSBASE "Checks whether the returned fs base value is the same using rdfsbase and prctl syscall."

#endif
