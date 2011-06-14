#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <deque>
#include <queue>

#include "packet.hpp"

using namespace std;

class Event {
public:
  double time;
  int addr;

  bool operator() ( const Event &lhs, const Event &rhs ) const
  {
    return lhs.time > rhs.time;
  }

  Event( double s_time, int s_addr ) : time( s_time ), addr( s_addr ) {}
  Event( void ) : time( -1 ), addr( -1 ) {}
};

typedef priority_queue<Event, deque<Event>, Event> event_queue_t;

class Container {
protected:
  event_queue_t wakeups;

public:
  Container() : wakeups() {}
  Container( const Container &x ) : wakeups( x.wakeups ) {}
  virtual ~Container() {}

  virtual void sleep_until( double time, int source_addr ) = 0;
  virtual void signal_sendable( int source_addr ) = 0; /* Channel tells container it is sendable */
  virtual bool can_send( int source_addr ) = 0; /* Channel asks container whether it can send */
  virtual void receive( int source_addr, Packet p ) = 0; /* Channel sends packet to container */
  virtual double time( void ) = 0; /* Channel asks for current time */
};

#endif
