#include <iostream>
#include <assert.h>

#include "throughput_channel.hpp"

ThroughputChannel::ThroughputChannel( Time *tick, double s_throughput )
  : Channel( tick ), throughput( s_throughput ), next_free_time( 0 )
{}

void ThroughputChannel::send( Packet pack )
{
  assert( is_free() ); /* can't send if busy */

  if ( dest ) {
    dest->send( pack );
  }

  next_free_time = time->now() + (double) pack.length / throughput;
  time->sleep_until( Event( next_free_time, this ) );
}

void ThroughputChannel::wakeup( void )
{
  if ( is_free() && src ) {
    src->wakeup();
  }
}
