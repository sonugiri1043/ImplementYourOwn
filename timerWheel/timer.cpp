/**
 * Simple Timer Wheel Implementation
 *
 * Compile using 'g++ timer.cpp -o timer --std=C++11'
 */

#include <iostream>
#include <functional> /* for function */
#include <unistd.h>   /* for sleep    */

using namespace std;

/**
 * ARRAY OF LIST USED FOR TIMER INTERVALS UPTO MAXIMUM INTERVAL
 *                        _______
 *     Element 0         |_______|
 *     Element 1         |_______|
 *                       |_______|
 *                       |_______|---> List of timer to expire at this time
 *                       |_______|
 *                       |_______|  
 *                       |_______|
 *        currentTime -->|_______|
 *                       |_______|
 *                       |_______|
 *     Element           |_______| 
 *     MaxTimeInterval - 1 
 *
 *
 * Basic Scheme for Timer Intervals within a Specified Range
 *
 * A basic timing wheel approach is applicable when all timers have a
 * maximum period of no more than MaxInterval. The modified algorithm
 * takes O(1) latency for START_TIMER, STOP_TIMER, and PER_TICK_BOOKKEEPING.
 *
 * Let the the granularity be 1 unit. Then current time is represented by a
 * pointer to an element in a circular buffer with dimensions [0, MaxInterval - 1 ].
 * To set a timer at j units past current time, we index into element
 * ( i + j mod MaxInterval), and put the timer at the head of a list of timers
 * that will expire at a time = CurrentTime + j units. Each tick increment the
 * current timer pointer ( mod maxInterval ) and check the array element being
 * pointer to. If the element is 0 ( no list of timers waiting to expire ), not
 * more work is done on that timer tick. But if it is non-zero, we do EXPIRE_PROCESSING
 * on all timers that are stored in that list. Thus the latency for START_TIMER
 * is O(1); PER_TICK_BOOKKEEPING is O(1). If the timer lists are doubly linked,
 * we store a pointer to each timer record, then the latency of STOP_TIMER is
 * also O(1).
 */

/* Used in list of timers */
struct Timer {
  function< void( int ) > action;
  int requestId; /* to uniquely identify this timer */
  Timer *next;
};

/* TimerWheel is an array of TwElement */
struct TwElement {
  Timer *head;
};

class TimerWheel {
private:
  int granularity;
  int maxTimeInterval;
  int currentTime;
  
  TwElement *tw; /* pointer to timer wheel array */
  
  void perTickBookkeeping();
  void expiryProcessing( function< void(int ) > action );
  
public:
  TimerWheel( int maxTimeInterval, int granularity );
  void startClock();
  void startTimer( int interval, int requestId, function< void( int ) > action );
  void stopTimer( int requestId );
};

/**
 * Initialize Timer Wheel data structure.
 * @maxTimeInterval: Range of timer ( currentTime, currentTime + maxIntervalTime )
 * @granularity    : Clock ticks after every granularity unit of time
 *
 * No of elements in array = maxTimeInterval / granularity
 */
TimerWheel::TimerWheel( int maxTimeInterval, int granularity ) {
  this->maxTimeInterval = maxTimeInterval;
  this->granularity = granularity;
  this->currentTime = 0;
      
  int noOfSlots = maxTimeInterval / granularity;
  tw = ( TwElement * ) calloc( noOfSlots, sizeof( TwElement ) );
  for( int i = 0; i < noOfSlots; i++ ) {
    tw[i].head = NULL;
  }
}

/**
 * Event clock, every tick perTickBookkeeping is called. 
 */
void TimerWheel::startClock() {
  while( true ) {
    this->currentTime = ( this->currentTime + this->granularity ) %	\
      this->maxTimeInterval;
    sleep( this->granularity );
    perTickBookkeeping();
  }
}

/**
 * Let the granularity of the timer be T units. Then every T units
 * this routine checks whether any outstanding timers have expired;
 * if so, it calls stopTimer which in turn invokes expiry processing.
 */
void TimerWheel::perTickBookkeeping() {
   while( tw[ currentTime ].head ) {
      // Pop the Timer in list and execute call back function.
      cout<< "Processing events scheduled at " << this->currentTime << endl;
      Timer *tmp;
      Timer *current = tw[ currentTime ].head;
      while( current != NULL ) {
	expiryProcessing( current->action );
	tmp = current->next;
	free( current );
	current = tmp;
      }
      tw[ currentTime ].head = NULL;
   }
}

/**
 * Calls callback function associated with timer.
 */
void TimerWheel::expiryProcessing( function< void( int ) > action ) {
  action( this->currentTime );
}

/**
 * Start a timer that will expire after "interval" units of time.
 * 'requestId' is used to distinguish this timer from other timers that client
 * has outstanding.
 * 'action' is the call back function that should gets called at timer expiration.
 */
void TimerWheel::startTimer( int interval, int requestId, function< void( int ) > action ) {
   int index = ( this->currentTime + interval ) % this->maxTimeInterval;
   Timer *timer = ( Timer * ) malloc( sizeof( Timer ) );
   timer->next = tw[ index ].head;
   timer->action = action;
   tw[ index ].head = timer;
   return;
}

/**
 * Used requestId to locate the timer and stop it.
 */
void TimerWheel::stopTimer( int requestId ) {
  cout<< "Removing timer with request id " << requestId << endl;
  /*
    Not implemente now, can easily be done by keeping a map bw requestId and timer node.
    Delete the mapping when the timer expires.
    
    #include <map>
    ...
    // in TimerWheel class
    map<int, Timer *> requestIdToTimerMap;
    ...
    // In startTimer function
    requestIdToTimerMap[ requestId ] = timer;
    ...
    // In perTickBookkeeping
    del requestIdToTimerMap[ requestId ];
    ...
    // In stopTimer, delete the node from the link list, also delete the mapping
   */
}

/**
 * Callback function to be called at timeout, used for demo purpose.
 */
void callBackFn( int time ) {
  cout<< "Callback function called at " << time << endl;
}

int main() {
   // Timerwheel with maxInterval 100 sec and granulariry 1 sec
   TimerWheel *tw = new TimerWheel( 100, 1 );
   tw->startTimer( 10, 1, callBackFn );
   tw->startTimer( 10, 2, callBackFn );
   tw->startTimer( 15, 3, callBackFn );
   tw->startTimer( 40, 4, callBackFn );
   tw->startTimer( 45, 4, callBackFn );

   tw->startClock();

   tw->stopTimer( 4 );

   return 0;
}
