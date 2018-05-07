#ifndef _TEST_RIPFXSAVE_H
#define _TEST_RIPFXSAVE_H

#include "jitmenot.h"

int register_test_ripfxsave(struct test_chain *);

#define FXSAVE_MEMORY_SIZE 0x34

#define TEST_ID_RIPFXSAVE 6
#define TEST_NAME_RIPFXSAVE "ripfxsave"
#define TEST_DESC_RIPFXSAVE "Executes fxsave instruction and checks the saved instruction pointer value."

#endif
