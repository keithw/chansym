#include "pinger.hpp"

Pinger::Pinger( Time *time, Channel *s_channel, double s_inc )
  : Actor( time ), channel( s_channel ),
    next_ping_time( time->now() ), increment( s_inc ), counter( 0 )
{
  wakeup();
}

void Pinger::wakeup( void )
{
  if ( time->now() >= next_ping_time ) {
    channel->send( Packet( counter, 12000 ) );
    counter++;
    next_ping_time += increment;
    time->sleep_until( Event( next_ping_time, this ) );
  }
}
