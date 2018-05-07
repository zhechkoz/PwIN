#ifndef _TEST_ENTER_H
#define _TEST_ENTER_H

#include "jitmenot.h"

int register_test_enter(struct test_chain *);

#define TEST_ID_ENTER 12
#define TEST_NAME_ENTER "enter"
#define TEST_DESC_ENTER "Checks whether the enter x86 instruction is legal and can be executed."

#endif
