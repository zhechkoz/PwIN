#ifndef _TEST_RIPSYSCALL_H
#define _TEST_RIPSYSCALL_H

#include "jitmenot.h"

int register_test_ripsyscall(struct test_chain *);

#define TEST_ID_RIPSYSCALL 8
#define TEST_NAME_RIPSYSCALL "ripsyscall"
#define TEST_DESC_RIPSYSCALL "Checks whether rip value is saved in rcx register after syscall."

#endif
