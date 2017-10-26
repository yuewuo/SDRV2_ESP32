import socket

ip_port = ('192.168.43.99', 80)
sk = socket.socket()
sk.connect(ip_port)
sk.sendall(bytes('GET / HTTP/1.0\r\n\r\n'))
server_rply = sk.recv(1024)
print str(server_rply)
sk.close()
