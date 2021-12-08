import socket

ip = socket.gethostbyname(socket.gethostname())
port = 8080

mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
mysock.connect((ip, port))
cmd = 'POST / HTTP/1.1'.encode()
mysock.send(cmd)
