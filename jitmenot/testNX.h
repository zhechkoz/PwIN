#ifndef _TEST_NX_H
#define _TEST_NX_H

#include "jitmenot.h"

int register_test_testnx(struct test_chain *);

#define ASSEMBLY_SIZE 13

#define TEST_ID_NX 3
#define TEST_NAME_NX "nx"
#define TEST_DESC_NX "Attempts to execute code on a page with non-executable permissions."

#endif
