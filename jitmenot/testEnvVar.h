#ifndef _TEST_ENVVAR_H
#define _TEST_ENVVAR_H

#include "jitmenot.h"

int register_test_envvar(struct test_chain *);

#define NUM_PIN_ENV_VAR 5
#define NUM_DR_ENV_VAR 3
#define NUM_DYNINST_ENV_VAR 1

#define TEST_ID_ENVVAR 0
#define TEST_NAME_ENVVAR "envvar"
#define TEST_DESC_ENVVAR "Checks for PIN specific environment variables."

#endif
