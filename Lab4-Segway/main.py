""" Simple script to send a UDP packet to a specific IP and port """
import socket

UDP_IP="172.20.10.100"
UDP_PORT=8888
MESSAGE=b"f"

def main():
    """
    Main method
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

if __name__ == "__main__":
    main()
