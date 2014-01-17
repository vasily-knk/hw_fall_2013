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
    typedef uint64_t task_id_t;
    
    enum cancel_result_t
    {
        NOT_FOUND,
        REMOVED_FROM_QUEUE,
        TERMINATED
    };

private:
    typedef boost::thread thread_t;
    typedef shared_ptr<boost::thread> thread_ptr;

    typedef boost::mutex mutex_t;
    typedef boost::mutex::scoped_lock mutex_lock_t;

    typedef size_t thread_id_t;

private:

/*    struct task_assignation
    {
    task_assignation(threadpool *owner, thread_id_t thread_id, pt::time_duration time)
        : owner_(owner)
    {
        task_data_ = owner_->assign_task(thread_id, time);
    }
    
    const pair<task_id_t, task_t> &task_data() const
    {
        return task_data_
    }

    ~task_assignation()
    {
        owner_->unassign_task(task_data_->first);
    }

    private:
        threadpool *owner_;
        optional<pair<task_id_t, task_t>> task_data_;
    };*/

public:

    threadpool(size_t num_threads)
        : next_task_id_(0)
        , next_thread_id_(0)
        , time_to_die_(false)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            auto t = make_shared<boost::thread>(boost::bind(&threadpool::thread_run, this, i, true));
            threads_.insert(make_pair(next_thread_id_++, t));
        }
    }

    ~threadpool()
    {
        clear_queue();
        
        time_to_die_ = true;
        tasks_cond_.notify_all();

        BOOST_FOREACH(const auto &t, threads_)
            t.second->join();
    }

public:
    task_id_t add_task(task_t task)
    {
        mutex_lock_t lock(tasks_mutex_);
        const task_id_t task_id = next_task_id_++;
        tasks_threads_.insert(make_pair(task_id, boost::none));
        tasks_queue_.push(make_pair(task_id, task));
        
        tasks_cond_.notify_one();
        return task_id;
    }

    cancel_result_t cancel_task(task_id_t task_id)
    {
        mutex_lock_t threads_lock(tasks_mutex_);
        if (!tasks_threads_.count(task_id) || tasks_to_cancel_.count(task_id))
            return NOT_FOUND;

        if (auto thread_id = tasks_threads_.at(task_id))
        {
            threads_.at(*thread_id)->interrupt();
            return TERMINATED;
        }
        else
        {
            MY_ASSERT(!tasks_to_cancel_.count(task_id));
            tasks_to_cancel_.insert(task_id);
            return REMOVED_FROM_QUEUE;
        }
    }

private:
    void thread_run(thread_id_t thread_id, bool hot)
    {
        while (!time_to_die_)
        {
            try
            {
                boost::this_thread::interruption_point();
            }
            catch (boost::thread_interrupted const&)
            {
                mutex_lock_t l(cout_mutex);
                cout << "Thread " << thread_id << " had a late interruption" << endl;
            }

            const auto task = assign_task(thread_id, boost::none);
            if (!task)
            {
                MY_ASSERT(!hot || time_to_die_);
                break;
            }

            {
                mutex_lock_t l(cout_mutex);
                cout << "Task " << task->first << " executed on thread " << thread_id << endl;
            }

            run_task(thread_id, task->second);

            unassign_task(thread_id, task->first);

            {
                mutex_lock_t l(cout_mutex);
                cout << "Task " << task->first << " finished on thread " << thread_id << endl;
            }
        }

        {
            mutex_lock_t l(cout_mutex);
            cout << "Thread " << thread_id << " finished" << endl;
        }
    }

    void run_task(thread_id_t thread_id, task_t task)
    {
        try
        {
            // run the task
            task();
        }
        catch (boost::thread_interrupted const&)
        {
            mutex_lock_t l(cout_mutex);
            cout << "Thread " << thread_id << " interrupted" << endl;
        }
    }

    optional<pair<uint64_t, task_t>> assign_task(thread_id_t thread_id, optional<pt::time_duration> time)
    {
        mutex_lock_t lock(tasks_mutex_);
        optional<pair<uint64_t, task_t>> res;
        
        while (!res)
        {
            if (tasks_queue_.empty())
            {
                if (time)
                    tasks_cond_.timed_wait(lock, *time);
                else
                    tasks_cond_.wait(lock);
            }

            if (tasks_queue_.empty())
            {
                MY_ASSERT(time || time_to_die_);
                return boost::none;
            }

            MY_ASSERT(!tasks_queue_.empty());
            res = tasks_queue_.front();
            tasks_queue_.pop();

            if (tasks_to_cancel_.count(res->first))
            {
                tasks_to_cancel_.erase(res->first);
                res = boost::none;
            }
        }

        MY_ASSERT(!tasks_threads_.at(res->first));
        tasks_threads_.at(res->first) = thread_id;

        return res;
    }

    void unassign_task(thread_id_t thread_id, task_id_t task_id)
    {
        mutex_lock_t lock(tasks_mutex_);
        MY_ASSERT(tasks_threads_.at(task_id) == thread_id);
        tasks_threads_.erase(task_id);
    }

    void clear_queue()
    {
        mutex_lock_t lock(tasks_mutex_);
        tasks_queue_.swap(queue<pair<uint64_t, task_t>>());
    }

private:
    unordered_map<task_id_t, optional<thread_id_t>> tasks_threads_;
    queue<pair<uint64_t, task_t>> tasks_queue_;
    unordered_set<task_id_t> tasks_to_cancel_;
    mutex_t tasks_mutex_;
    boost::condition_variable tasks_cond_;
    
    unordered_map<thread_id_t, thread_ptr> threads_;
    
    task_id_t next_task_id_;
    thread_id_t next_thread_id_;

    boost::atomic_bool time_to_die_;
};