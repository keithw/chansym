#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <stdio.h>
#include <deque>
#include "packet.hpp"

using namespace std;

#include "my_queue.h"
#include "channel.hpp"

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

class Container {
protected:
  peekable_priority_queue<Event, deque<Event>, Event> wakeups;

public:
  Container() : wakeups() {}
  Container( const Container &x ) : wakeups( x.wakeups ) {}
  virtual ~Container() {}

  virtual void sleep_until( double time, int source_addr ) = 0;
  virtual void signal_sendable( int source_addr ) = 0; /* Channel tells container it is sendable */
  virtual bool can_send( int source_addr ) = 0; /* Channel asks container whether it can send */
  virtual void receive( int source_addr, Packet p ) = 0; /* Channel sends packet to container */
  virtual double time( void ) = 0; /* Channel asks for current time */

  virtual void fork( int source_addr, double my_probability, Channel *other, Channel::ForkState *fs ) = 0;
  virtual double probability( int source_addr ) = 0;
};

#endif
