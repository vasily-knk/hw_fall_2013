all: client server

CFLAGS=-std=c++0x -lboost_filesystem -lpthread -lboost_thread -lboost_system


client: client.cpp
	g++ $(CFLAGS) client.cpp -o client

server: server.cpp
	g++ $(CFLAGS) server.cpp -o server

clean:
	rm -rf client server
