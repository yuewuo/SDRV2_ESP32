import socket

ip_port = ('127.0.0.1', 666)
web = socket.socket()
web.bind(ip_port)
web.listen(5)
if True:
	print "listening!"
	conn, addr = web.accept()
	data = conn.recv(1024)
	print "receving:", data
	conn.send(bytes("GET http://60.205.187.189 HTTP/1.0\r\n\r\n"))

