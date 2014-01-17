#include "stdafx.h"
#include "threadpool.h"

#include <signal.h>

boost::scoped_ptr<threadpool> pool;
boost::mutex cout_mutex;

struct args_t
{
    size_t num_hot_threads;
    pt::time_duration timeout;
};

optional<args_t> parse_args(int argc, char* argv[])
{
    args_t res;
    bool error = true;
    
    if (argc == 3)
    {
        try
        {
            res.num_hot_threads = boost::lexical_cast<size_t>(argv[1]);
            res.timeout = pt::seconds(boost::lexical_cast<size_t>(argv[2]));
            error = false;
        }
        catch (boost::bad_lexical_cast &) {}
    }

    if (error)
    {
        cout << "Usage: program num_hot_threads timeout" << endl;
        return boost::none;
    }
    return res;
}

void sig_handler(int /*signum*/)
{
    pool.reset();
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    auto args = parse_args(argc, argv);
    if (!args)
        return 1;

    pool.reset(new threadpool(args->num_hot_threads, args->timeout));

    while (pool)
    {
        string cmd;
        std::getline(std::cin, cmd);
        if (!pool)
            break;

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
                    auto task_id = pool->add_task(sleep_task(seconds));

                    boost::mutex::scoped_lock l(cout_mutex);
                    cout << "Task id: " << task_id << endl;
                    error = false;
                }
                else if (parts.at(0) == "cancel")
                {
                    auto task_id = boost::lexical_cast<threadpool::task_id_t>(parts.at(1));
                    auto res = pool->cancel_task(task_id);

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

