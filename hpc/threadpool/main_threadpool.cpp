#include "stdafx.h"
#include "threadpool.h"


boost::mutex cout_mutex;

int main(int argc, char* argv[])
{
	threadpool pool(2);

    while (true)
    {
        string cmd;
        std::getline(std::cin, cmd);

        vector<string> parts;

        boost::split(parts, cmd, boost::is_space());
        bool error = true;
        
        if (parts.size() == 2)
        {
            try
            {
                if (parts.at(0) == "add")
                {
                    int seconds = boost::lexical_cast<int>(parts.at(1));
                    auto task_id = pool.add_task(sleep_task(seconds));

                    boost::mutex::scoped_lock l(cout_mutex);
                    cout << "Task id: " << task_id << endl;
                    error = false;
                }
                else if (parts.at(0) == "cancel")
                {
                    auto task_id = boost::lexical_cast<threadpool::task_id_t>(parts.at(1));
                    auto res = pool.cancel_task(task_id);

                    boost::mutex::scoped_lock l(cout_mutex);
                    cout << "Task " << task_id << " ";
                    switch (res)
                    {
                    case threadpool::NOT_FOUND:
                        cout << "NOT_FOUND";
                        break;
                    case threadpool::REMOVED_FROM_QUEUE:
                        cout << "REMOVED_FROM_QUEUE";
                        break;
                    case threadpool::TERMINATED:
                        cout << "TERMINATED";
                        break;
                    }
                    cout << endl;
                    error = false;
                }
            } 
            catch (boost::bad_lexical_cast &)
            {
            }
        }
        

        if (error)
        {
            boost::mutex::scoped_lock l(cout_mutex);
            cout << "Error" << endl;
        }
    }

    return 0;
}

