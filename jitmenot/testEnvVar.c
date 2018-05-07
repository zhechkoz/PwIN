#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jitmenot.h"
#include "testEnvVar.h"

static const char *pin_env_var[NUM_PIN_ENV_VAR] = {"PIN_INJECTOR64_LD_LIBRARY_PATH", "PIN_INJECTOR32_LD_LIBRARY_PATH",
                                                   "PIN_VM64_LD_LIBRARY_PATH", "PIN_VM32_LD_LIBRARY_PATH",
                                                   "PIN_CRT_TZDATA"};

static const char *dr_env_var[NUM_DR_ENV_VAR] = {"DYNAMORIO_CONFIGDIR", "DYNAMORIO_TAKEOVER_IN_INIT",
                                                 "DYNAMORIO_EXE_PATH"};

static const char *dyninst_env_var[NUM_DYNINST_ENV_VAR] = {"DYNINSTAPI_RT_LIB"};

int check_any_env(const char **env_var, int length) {
    for (int i = 0; i < length; i++) {
        char *value = getenv(env_var[i]);
#ifdef DEBUG
        printf("%s\t%s\n", env_var[i], value);
#endif
        if (value != NULL) return 1;
    }

    return 0;
}

int check_ld_preload(char *needle) {
    char *ld_preload = "LD_PRELOAD";
    char *value = getenv(ld_preload);
#ifdef DEBUG
    printf("%s\t%s\n", ld_preload, value);
#endif

    if (value == NULL) return 0;
    if (strstr(value, needle) != NULL) return 1;
    return 0;
}

static int detect(void) {
    int detected = 0;

    detected = check_any_env(pin_env_var, NUM_PIN_ENV_VAR) || detected;
    detected = check_any_env(dr_env_var, NUM_DR_ENV_VAR) || detected;
    detected = check_any_env(dyninst_env_var, NUM_DYNINST_ENV_VAR) || detected;
    detected = check_ld_preload("vgpreload") || detected;

    return detected ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_envvar(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_ENVVAR;

    test->detect = detect;
    test->description = TEST_DESC_ENVVAR;
    test->name = TEST_NAME_ENVVAR;
    test->cleanup = cleanup;

    return 0;
}
