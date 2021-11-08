#!/usr/bin/python3

import socket
import time

UDP_IP = "127.0.0.1"
UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
while True:
    time.sleep(1)
    sock.sendto(b"hi", (UDP_IP, UDP_PORT) )
    print(f"sent ")

