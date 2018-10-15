/*
 * Thread pool
 *
 * Compile with "g++ ThreadPool.cpp -o ThreadPool --std=c++11"
 */

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

class ThreadPool {
private:
  static int num_threads;        /* fixed no of threads in pool */
  static ThreadPool *threadPool; /* part of singleton pattern   */
  
  mutex job_queue_mutex;
  condition_variable conditionEmptyQueue;

  vector< thread > threads;
  queue< function<void()> > job_queue;

  ThreadPool() {
    cout<< "Creating " << num_threads << " in thread pool" << endl;
    for( int i = 0; i < num_threads; i++ ) {
      threads.push_back( thread( &ThreadPool::infinite_loop_fn, this ) );
    }
  }

  ~ThreadPool() {
    // for singleton class it only gets called at program termination.
  }

  void infinite_loop_fn();
public:
  static ThreadPool* getInstance() {
    if( threadPool == NULL ) {
      threadPool = new ThreadPool();
    }
    return threadPool;
  }
  
  void addJob( function< void() > job );
};

/* Initialize static class attributes */
int ThreadPool::num_threads = thread::hardware_concurrency();
ThreadPool* ThreadPool::threadPool = NULL;

/*
 * Threads run with an infinite loop, constantly waiting for new tasks
 * to grab and run.
 */
void ThreadPool::infinite_loop_fn() {
  function< void() > job;
  while( true ) {
    {
      unique_lock<mutex> lock( job_queue_mutex );
      while( job_queue.empty() ) {
	// lock will be released automatically.
	conditionEmptyQueue.wait( lock );
      }
      job = job_queue.front();
      job_queue.pop();
      // release the lock
    }
    job();
  }
}

void ThreadPool::addJob( function< void() > job ) {
  unique_lock<mutex> lock( job_queue_mutex );
  job_queue.push( job );
  conditionEmptyQueue.notify_one();
}

mutex display_mutex;

/* function that will be passed to job queue to be execute by thread pool */
void func() {
  display_mutex.lock();
  cout<< "Executing task in thread " << this_thread::get_id() << endl;
  display_mutex.unlock();
}


int main() {
  cout << "Thread pool implementation" << endl;
  ThreadPool* threadPool = ThreadPool::getInstance();

  // Add jobs to queue to be executed by thread pool
  for( int i = 0; i < 100; i++ ) {
    threadPool->addJob( func );
  }
  return 0;
}

/*
  Suggestions:
  Instead of creating threads in the pool in constructor we can create
  the threads in main function dynamically and push it into threadpool.
 */
