all: client server

CFLAGS=-std=c++0x -lboost_filesystem -lpthread -lboost_thread -lboost_system


client: client.cpp
	g++ client.cpp $(CFLAGS) -o client

server: server.cpp
	g++ server.cpp $(CFLAGS) -o server

clean:
	rm -rf client server
	
