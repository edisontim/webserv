all:
	clang++ -o server server.cpp
	clang++ -o client client.cpp

fclean:
	rm server client
