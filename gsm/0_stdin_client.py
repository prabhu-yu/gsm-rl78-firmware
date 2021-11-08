#!/usr/bin/python3
import socket

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 5555       # The port used by the server

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((HOST, PORT))

while True:
    command = input('enter the command: ')
    command = bytes(command, 'utf-8')
    s.sendall(command)
    print(f'sent {command}')



