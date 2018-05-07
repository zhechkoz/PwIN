#ifndef _TEST_SMC_H
#define _TEST_SMC_H

#include "jitmenot.h"

int register_test_smc(struct test_chain *);

#define TEST_ID_SMC 9
#define TEST_NAME_SMC "smc"
#define TEST_DESC_SMC "Checks whether self modifying code is detected."

#endif
