#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* See feature_test_macros(7) */
#endif

#include <stdio.h>
#include <stdlib.h>

#include "jitmenot.h"
#include "pmparser.h"
#include "testVMLeave.h"

static int detect(void) {
    unsigned char needle[VMLEAVE_SIZE] = {
        0x9d,                   // popfq
        0x48, 0x8B, 0x38,       // mov     rdi, [rax]
        0x48, 0x8B, 0x70, 0x08, // mov     rsi, [rax+8]
        0x48, 0x8B, 0x68, 0x10, // mov     rbp, [rax+10h]
        0x48, 0x8B, 0x60, 0x18, // mov     rsp, [rax+18h]
        0x48, 0x8B, 0x58, 0x20, // mov     rbx, [rax+20h]
        0x48, 0x8B, 0x50, 0x28, // mov     rdx, [rax+28h]
        0x48, 0x8B, 0x48, 0x30, // mov     rcx, [rax+30h]
        0x4C, 0x8B, 0x40, 0x40, // mov     r8, [rax+40h]
        0x4C, 0x8B, 0x48, 0x48, // mov     r9, [rax+48h]
        0x4C, 0x8B, 0x50, 0x50, // mov     r10, [rax+50h]
        0x4C, 0x8B, 0x58, 0x58, // mov     r11, [rax+58h]
        0x4C, 0x8B, 0x60, 0x60, // mov     r12, [rax+60h]
        0x4C, 0x8B, 0x68, 0x68, // mov     r13, [rax+68h]
        0x4C, 0x8B, 0x70, 0x70, // mov     r14, [rax+70h]
        0x4C, 0x8B, 0x78, 0x78  // mov     r15, [rax+78h]
    };

    int detected = 0;
    procmaps_struct *maps = pmparser_parse(-1); // this process
    if (maps == NULL) {
#ifdef DEBUG
        printf("Cannot parse the memory map!\n");
#endif
        return RESULT_UNK;
    }

    procmaps_struct *maps_tmp = NULL;

    while ((maps_tmp = pmparser_next()) != NULL) {
        if (maps_tmp->is_r && maps_tmp->is_x) {
            if (memmem(maps_tmp->addr_start, maps_tmp->length, needle, VMLEAVE_SIZE) != NULL) {
                detected = 1;
                break;
            }
        }
    }

    pmparser_free(maps);

    return detected == 1 ? RESULT_YES : RESULT_NO;
}

static int cleanup(void) {
    /* Nothing to be done */
    return 0;
}

int register_test_vmleave(struct test_chain *all_tests) {
    struct test_chain *test;

    test = test_chain_alloc_new(all_tests);

    if (!test) return 1 << TEST_ID_VMLEAVE;

    test->detect = detect;
    test->description = TEST_DESC_VMLEAVE;
    test->name = TEST_NAME_VMLEAVE;
    test->cleanup = cleanup;

    return 0;
}
