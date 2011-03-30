#include <assert.h>

#include "time.hpp"

Time::Time( void )
  : time( 0 ), wakeups()
{}

bool Time::tick( void )
{
  if ( wakeups.empty() ) {
    return false;
  }

  Event next_event = wakeups.top();
  wakeups.pop();

  assert( next_event.time >= time );
  time = next_event.time;

  assert( next_event.act );
  
  next_event.act->wakeup();

  return true;
}

void Time::sleep_until( Event e )
{
  assert( e.time >= time );
  wakeups.push( e );
}
