#ifndef TIME_HPP
#define TIME_HPP

#include <deque>
#include <queue>

#include "actor.hpp"

using namespace std;

class Event {
public:
  double time;
  Actor *act;

  bool operator() ( const Event &lhs, const Event &rhs ) const
  {
    return lhs.time > rhs.time;
  }

  Event( double s_time, Actor *s_act ) : time( s_time ), act( s_act ) {}
  Event( void ) : time( -1 ), act( NULL ) {}
};

typedef priority_queue<Event, deque<Event>, Event> event_queue_t;

class Time {
private:
  double time;
  event_queue_t wakeups;

public:
  bool tick( void );
  void sleep_until( Event e );

  double now( void ) { return time; }

  Time( void );
};

#endif
