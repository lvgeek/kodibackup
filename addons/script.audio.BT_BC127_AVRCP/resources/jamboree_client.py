import socket
import sys

'''
seek_right
seek_left
tune_xx.x
volume_xx
toggle_x
get_frequency
'''

UDP_IP = "127.0.0.1"
UDP_PORT = 5005
MESSAGE = sys.argv[1]+'_'+sys.argv[2]

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
data, addr = sock.recvfrom(1024)
sock.close()
print data

