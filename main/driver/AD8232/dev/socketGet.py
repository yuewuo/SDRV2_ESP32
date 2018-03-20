import socket

ip_port = ('192.168.43.78', 8232)
sk = socket.socket()
sk.connect(ip_port)

server_rply = sk.recv(1024)
print str(server_rply)
sk.close()
