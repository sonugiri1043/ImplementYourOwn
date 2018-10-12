/*
 * Object Pool Design Pattern
 *
 * Compile using "g++ pool.cpp -o pool --std=c++11"
 *
 */

#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

using namespace std;

#define TRACE

#ifdef TRACE
#define trace( x ) std::cout << x << endl
#else
#define trace( x ) 
#endif 

/* Resource to be pooled */
class Foo {
public:
  int data;
  void doTask( int i ) {
    trace( "Foo object, arg " << i );
  }
};

template <class T> class ObjectPool {

private:
  static ObjectPool<T> *pool;

  /* limits the no of resources to be created */
  static const unsigned int numObj = 10;

  mutex mu;
  condition_variable c;

  list<T *> idleObjects;
  list<T *> inUseObjects;

  ObjectPool() {
    // private constructor, follows singleton design pattern
    trace( "Instantiating resources to be pooled" );
    for( int i = 0 ; i < numObj; i++ ) {
      idleObjects.push_front( new T );
    }
  }

public:
  /*
    Static method to access class instance.
    Part of singleton design pattern.
    
    @return ObjectPool instance
   */
  static ObjectPool<T> * getInstance() {
    if( pool == NULL ) {
      pool = new ObjectPool();
    }
    return pool;
  }

  /*
    Returns resource instance
   */
  T* acquire() {
    unique_lock<mutex> lock( mu );
    while( idleObjects.empty() ) {
      // wait operations atomically release the mutex lock
      // and suspend the execution of the thread.
      trace( "Waiting for resource to become available!" );
      c.wait( lock );
    }

    T *obj = idleObjects.front();
    idleObjects.pop_front();
    inUseObjects.push_front( obj );
    trace( "Acquiring resource" );
    return obj;
  }

  /*
    Returns resource back to the pool.
   */
  void release( T *obj ) {
    unique_lock<mutex> lock( mu );
    trace( "Releasing resource!" );
    inUseObjects.remove( obj );
    idleObjects.push_front( obj );
    c.notify_one();
  }
};

template<> ObjectPool< Foo > *ObjectPool< Foo >::pool=NULL;

void client( int id ) {
  ObjectPool< Foo > *objPool = ObjectPool< Foo >::getInstance();

  Foo *node = objPool->acquire();
  node->doTask( id );
  this_thread::sleep_for(std::chrono::seconds( 10 ));
  objPool->release( node );
}

int main() {
  cout<< "Basic object( resource ) pool implementation" << endl;
  
  int numThreads = 12;
  thread threads[ 12 ];

  for( int i=0; i < numThreads; i++ ) {
    // create more no of threads than resource to test wait and lock mechanism.
    threads[ i ] = thread( client, i );
  }

  for( int i=0; i < numThreads; i++ ) {
    if( threads[i].joinable() ) {
      threads[i].join();
    }
  }
  return 0;
}

/*
 * Future enhancements:
 *  inUseObjects list can be used to recover lost objects with the help
 *  of timeout. 
 */
