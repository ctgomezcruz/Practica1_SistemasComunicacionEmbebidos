# ITESO - Especialidad en Sistemas Embebidos #
# Sistemas de comunicacion para sistemas embebidos #
# Practica 1 - Cesar Tomas Gomez Cruz - Febrero 2023 #

import socket     # The TCP/IP Sockets module
import binascii   # For CRC32
from Crypto.Cipher import AES # For AES128

# AES Data #
key = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x00\x01\x02\x03\x04\x05\x06'
iv = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'

# Server IP Address and socket#
HOST = "192.168.0.100"
PORT = 10000  # Port to listen on (non-privileged ports are > 1023)

# Getting the decryptor #
decryptor = AES.new(key, AES.MODE_CBC, IV=iv)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    # Binding to the IP Address through the socket and listen for incoming data #
    s.bind((HOST, PORT))
    s.listen()

    # Accepting the incoming data #
    conn, addr = s.accept()

    with conn:
        print(f"Connected by {addr}")
        while True:
            # Receiving the datagrams coming from client #
            data = conn.recv(1024)
            if not data:
                break
            else:
                # Extracting the 4 bytes corresponding to CRC checksum #
                msg_crc = hex(int.from_bytes(data[-4:], 'little'))
                # Extracting the encrypted message from the datagram received #
                # Last 5 bytes are removed #
                msg = data[:-5]
                # Calculating the length of the message (removing the first 16 bytes) #
                length = int.from_bytes(data[-5:-4], 'big') - 16
                # Calculating the CRC checksum from the encrypted message received #
                calculated_crc = hex(binascii.crc32(msg))

                # Comparing the two CRC checksums #
                # One inserted in the datagram and one calculated in the server #
                if msg_crc == calculated_crc:
                    print("Message received without errors!")
                    print("Message encrypted: ", hex(int.from_bytes(data[:-5], 'big')))
                    print("CRC calculated from message received: ", calculated_crc)
                    # Decrypting the message #
                    dec_msg = decryptor.decrypt(msg)
                    # Removing the first 16 bytes (the ones that are wrong) #
                    dec_msg = dec_msg[16:length+16]
                    print("Message decrypted: ", dec_msg)
                else:
                    print("Message is corrupted!")
                    break

                # Sending back to the client the message decrypted as an echo #
                # conn.sendall(dec_msg)
                conn.sendall(data)
    # Closing the comm #
    s.close()