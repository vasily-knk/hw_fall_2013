all: threadpool

CFLAGS=-std=c++0x -lboost_filesystem -lpthread -lboost_thread -lboost_system

threadpool: main_threadpool.cpp
	g++ main_threadpool.cpp $(CFLAGS) -o threadpool

clean:
	rm -rf threadpool
	
