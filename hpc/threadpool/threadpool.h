#pragma once

extern boost::mutex cout_mutex;

struct sleep_task
{
    sleep_task(int seconds)
        : dur_(pt::seconds(seconds))
    {}

    void operator()() const
    {
        boost::this_thread::sleep(dur_);
    }

private:
    pt::time_duration dur_;
};

struct threadpool
{
typedef boost::function<void()> task_t;

private:
    typedef boost::thread thread_t;
    typedef shared_ptr<boost::thread> thread_ptr;
    typedef boost::thread::id thread_id_t;

    typedef boost::mutex mutex_t;
    typedef boost::mutex::scoped_lock mutex_lock_t;

public:

    threadpool(size_t num_threads)
        : main_thread_id_(boost::this_thread::get_id())
        , next_task_id_(0)
        , time_to_die_(false)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            auto t = make_shared<boost::thread>(boost::bind(&threadpool::thread_run, this, i));
            threads_[t->get_id()] = t;
        }
    }

    ~threadpool()
    {
        time_to_die_ = true;
        BOOST_FOREACH(const auto &t, threads_)
        {
            t.second->join();
        }
    }

public:
    void add_task(task_t task)
    {
        MY_ASSERT(boost::this_thread::get_id() == main_thread_id_);
        
        mutex_lock_t lock(tasks_mutex_);
        tasks_.push(make_pair(next_task_id_++, task));
        tasks_cond_.notify_one();
    }

private:
    void thread_run(size_t my_thread_id)
    {
        while (!time_to_die_)
        {
            const auto task = extract_task();


            {
                mutex_lock_t l(cout_mutex);
                cout << "Task " << task.first << " executed on thread " << my_thread_id << endl;
            }

            task.second();
            
            {
                mutex_lock_t l(cout_mutex);
                cout << "Task " << task.first << " finished on thread " << my_thread_id << endl;
            }
        }
        
        {
            mutex_lock_t l(cout_mutex);
            cout << "Thread " << my_thread_id << " finished" << endl;
        }
    }

    pair<uint64_t, task_t> extract_task()
    {
        mutex_lock_t lock(tasks_mutex_);

        if (tasks_.empty())
        {
            tasks_cond_.wait(lock);
        }

        MY_ASSERT(!tasks_.empty());
        const auto res = tasks_.front();
        tasks_.pop();
        return res;
    }


private:
    queue<pair<uint64_t, task_t>> tasks_;
    mutex_t tasks_mutex_;
    boost::condition_variable tasks_cond_;

    map<thread_id_t, thread_ptr> threads_;
    thread_id_t main_thread_id_;
    uint64_t next_task_id_;

    boost::atomic_bool time_to_die_;
};