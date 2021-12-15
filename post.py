import socket

# ip = socket.gethostbyname(socket.gethostname())
ip = "10.1.11.1"
port = 8080

mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
mysock.connect((ip, port))
# cmd = 'DELETE / HTTP/1.1'.encode()
mysock.send(cmd)
data = mysock.recv(1000)
print(data.decode())
