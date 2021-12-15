import socket
import signal
import os
import time
import sys

def child():
	signal.sigwait([signal.SIGUSR1])
	print("Process number " + os.getpid() + " is exiting now")
	sys.exit()


pids = []
pid_number = 2

for i in range(pid_number):
	# print(i)
	try :
		pid = os.fork()
	except OSError as error:
		print(error)
	if (pid == 0):
		child()
	else:
		pids.append(pid)


for j in range(pid_number):
	os.kill(pids[i], signal.SIGUSR1)

# ip = socket.gethostbyname(socket.gethostname())
# fd = open("stress_test.txt", "a")
# port = 8080

	
# mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# mysock.connect((ip, port))
# cmd = 'GET / HTTP/1.1'.encode()
# mysock.send(cmd)
# data = mysock.recv(1024)
# fd.write(data.decode('ASCII'))
# mysock.close()

# fd.close()
sys.exit()