#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* See feature_test_macros(7) */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jitmenot.h"
#include "pmparser.h"
#include "testVMLeave.h"

const unsigned char pin_vmleave[PIN_VMLEAVE_SIZE] = {
    0x9D,                   // popf
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

const unsigned char qbdi_vmleave[QBDI_VMLEAVE_SIZE] = {
    0x9D,                                     // popf
    0x48, 0x8B, 0x05, 0x3B, 0x12, 0x00, 0x00, // mov    rax, QWORD PTR [rip+0x123b]
    0x48, 0x8B, 0x1D, 0x3C, 0x12, 0x00, 0x00, // mov    rbx, QWORD PTR [rip+0x123c]
    0x48, 0x8B, 0x0D, 0x3D, 0x12, 0x00, 0x00, // mov    rcx, QWORD PTR [rip+0x123d]
    0x48, 0x8B, 0x15, 0x3E, 0x12, 0x00, 0x00, // mov    rdx, QWORD PTR [rip+0x123e]
    0x48, 0x8B, 0x35, 0x3F, 0x12, 0x00, 0x00, // mov    rsi, QWORD PTR [rip+0x123f]
    0x48, 0x8B, 0x3D, 0x40, 0x12, 0x00, 0x00, // mov    rdi, QWORD PTR [rip+0x1240]
    0x4C, 0x8B, 0x05, 0x41, 0x12, 0x00, 0x00, // mov    r8, QWORD PTR [rip+0x1241]
    0x4C, 0x8B, 0x0D, 0x42, 0x12, 0x00, 0x00, // mov    r9, QWORD PTR [rip+0x1242]
    0x4C, 0x8B, 0x15, 0x43, 0x12, 0x00, 0x00, // mov    r10, QWORD PTR [rip+0x1243]
    0x4C, 0x8B, 0x1D, 0x44, 0x12, 0x00, 0x00, // mov    r11, QWORD PTR [rip+0x1244]
    0x4C, 0x8B, 0x25, 0x45, 0x12, 0x00, 0x00, // mov    r12, QWORD PTR [rip+0x1245]
    0x4C, 0x8B, 0x2D, 0x46, 0x12, 0x00, 0x00, // mov    r13, QWORD PTR [rip+0x1246]
    0x4C, 0x8B, 0x35, 0x47, 0x12, 0x00, 0x00, // mov    r14, QWORD PTR [rip+0x1247]
    0x4C, 0x8B, 0x3D, 0x48, 0x12, 0x00, 0x00, // mov    r15, QWORD PTR [rip+0x1248]
    0x48, 0x8B, 0x2D, 0x49, 0x12, 0x00, 0x00, // mov    rbp, QWORD PTR [rip+0x1249]
    0x48, 0x8B, 0x25, 0x4A, 0x12, 0x00, 0x00  // mov    rsp, QWORD PTR [rip+0x124a]

};

const unsigned char *needles[NUM_PATTERNS] = {pin_vmleave, qbdi_vmleave};
const size_t needles_sizes[NUM_PATTERNS] = {PIN_VMLEAVE_SIZE, QBDI_VMLEAVE_SIZE};

static int detect(void) {
    int detected = 0;
    void *found = NULL;

    procmaps_struct *maps = pmparser_parse(-1); // this process
    if (maps == NULL) {
#ifdef DEBUG
        printf("Cannot parse the memory map!\n");
#endif
        return RESULT_UNK;
    }

    procmaps_struct *maps_tmp = NULL;

    while ((maps_tmp = pmparser_next()) != NULL) {
        if (maps_tmp->is_r && strstr(maps_tmp->pathname, "vvar") == NULL) {
            for (int i = 0; i < NUM_PATTERNS; i++) {
                if ((found = memmem(maps_tmp->addr_start, maps_tmp->length, needles[i], needles_sizes[i])) != NULL) {
                    detected += 1;
#ifdef DEBUG
                    printf("%p\n", found);
#endif
                }
            }
        }
    }

    pmparser_free(maps);
#ifdef DEBUG
    printf("VMLeave was found %d time(s) in memory\n", detected);
#endif

    // Normally VMLeave code can only be found in stack
    return detected > 2 * 1 ? RESULT_YES : RESULT_NO;
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
