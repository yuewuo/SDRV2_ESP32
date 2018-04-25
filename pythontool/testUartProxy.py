import socket, time

def getreply(info):
    ip_port = ('192.168.31.37', 80)
    sk = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sk.connect(ip_port)
    sk.sendall(bytes(info, encoding='utf-8'))
    server_rply = sk.recv(1024)
    print(str(server_rply))
    sk.close()

if __name__ == '__main__':
    starttime = time.time() * 1000
    # getreply('GET /exec/reboot HTTP/1.0\r\n\r\n')  # 重启
    # getreply('GET /exec/start%20proxy%20UART1 HTTP/1.0\r\n\r\n')
    getreply('GET /exec/souart%20clear%20777A1%20115A11%20flush HTTP/1.0\r\nESPProxyTo: UART1\r\nESPProxyWait: no-wait\r\n\r\n')
    print('proxy finish %d ms' % (time.time() * 1000 - starttime))
