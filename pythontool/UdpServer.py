from socket import *

ip_port = ('0.0.0.0', 10001)
bufsize = 1024
udpServer = socket(AF_INET,SOCK_DGRAM)
udpServer.bind(ip_port)

cnt = 0

while True:
    cnt += 1
    print("listening! %d" % cnt)
    data, addr = udpServer.recvfrom(bufsize)  #接收数据和返回地址
    print ("receving:", data)

