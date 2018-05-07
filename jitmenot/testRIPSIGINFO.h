#ifndef _TEST_RIPSIGINFO_H
#define _TEST_RIPSIGINFO_H

#include "jitmenot.h"

int register_test_testripsiginfo(struct test_chain *);

#define TEST_ID_RIPSIGINFO 7
#define TEST_NAME_RIPSIGINFO "ripsiginfo"
#define TEST_DESC_RIPSIGINFO "Causes an int3 and checks the saved instruction pointer value in FPREGS."

#endif
