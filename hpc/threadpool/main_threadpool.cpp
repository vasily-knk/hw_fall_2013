#include "stdafx.h"
#include "threadpool.h"


boost::mutex cout_mutex;

int main(int argc, char* argv[])
{
	threadpool pool(2);

    string cmd;
    while (cmd != "q")
    {
        std::cin >> cmd;
        if (cmd == "a")
            pool.add_task(sleep_task(5));
    }

    return 0;
}

