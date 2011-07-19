#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <stdio.h>
#include <deque>
#include <string>
#include <boost/functional/hash.hpp>

#include "packet.hpp"

using namespace std;

#include "my_queue.h"
#include "channel.hpp"

class Event {
public:
  double time;
  int addr;
  int sort_order;

  bool operator() ( const Event &lhs, const Event &rhs ) const
  {
    if ( lhs.time > rhs.time ) {
      return 1;
    } else if ( lhs.time < rhs.time ) {
      return 0;
    } else {
      return lhs.sort_order > rhs.sort_order;
    }
  }

  Event( double s_time, int s_addr, int s_order ) : time( s_time ), addr( s_addr ), sort_order( s_order ) {}
  Event( void ) : time( -1 ), addr( -1 ), sort_order( INT_MAX ) {}

  bool operator==( const Event &x ) const { return (time == x.time) && (addr == x.addr) && (sort_order == x.sort_order); }
  friend size_t hash_value( const Event & e ) {
    size_t seed = 0;
    boost::hash_combine( seed, e.time );
    boost::hash_combine( seed, e.addr );
    boost::hash_combine( seed, e.sort_order );
    return seed;
  }
};

class Container {
protected:
  peekable_priority_queue<Event, deque<Event>, Event> wakeups;

public:
  Container() : wakeups() {}
  Container( const Container &x ) : wakeups( x.wakeups ) {}
  virtual ~Container() {}

  virtual void sleep_until( double time, int source_addr, int sort_order=0 ) = 0;
  virtual void signal_sendable( int source_addr ) = 0; /* Channel tells container it is sendable */
  virtual bool can_send( int source_addr ) = 0; /* Channel asks container whether it can send */
  virtual void receive( int source_addr, Packet p ) = 0; /* Channel sends packet to container */
  virtual double time( void ) const = 0; /* Channel asks for current time */

  virtual void fork( int source_addr, double my_probability, Channel::ForkState *fs ) = 0;
  virtual double probability( int source_addr ) = 0;

  virtual string identify( void ) const = 0;

  typedef typename peekable_priority_queue<Event, deque<Event>, Event>::const_iterator wakeup_iterator;
  wakeup_iterator wakeup_begin( void ) { return wakeups.begin(); }
  wakeup_iterator wakeup_end( void ) { return wakeups.end(); }

  virtual void clear_wakeups( void ) = 0;
  virtual void clear_wakeups( int source_addr ) = 0;
};

#endif
