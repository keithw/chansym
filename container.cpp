#include <algorithm>

#include "container.hpp"
#include "close.hpp"

void Container::rationalize_wakeups( void )
{
  peekable_priority_queue<Event, deque<Event>, Event> new_wakeups;

  double now = time();

  for_each( wakeups.begin(), wakeups.end(),
	    [&new_wakeups, now]( const Event &x ) {
	      Event ne( quantize_time( x.time - now ),
			x.addr,
			x.sort_order );
	      new_wakeups.push( ne );
	    } );

  wakeups = new_wakeups;
}

wakeup_queue round_wakeups( const wakeup_queue &q )
{
  wakeup_queue new_wakeups;

  for_each( q.begin(), q.end(),
	    [&new_wakeups]( const Event &x ) {
	      Event ne( rounder( x.time ),
			x.addr,
			x.sort_order );
	      new_wakeups.push( ne );
	    } );

  return new_wakeups;
}
