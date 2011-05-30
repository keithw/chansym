#include "pinger.hpp"

Pinger::Pinger( Time *time, double s_inc )
  : Channel( time ),
    next_ping_time( time->now() ), increment( s_inc ), counter( 0 )
{
  wakeup();
}

void Pinger::wakeup( void )
{
  if ( time->now() >= next_ping_time ) {
    if ( dest ) {
      dest->send( Packet( 12000, 0, counter, time->now() ) );
    }
    counter++;
    next_ping_time += increment;
    time->sleep_until( Event( next_ping_time, this ) );
  }
}
