#include <iostream>
#include <assert.h>

#include "buffer_channel.hpp"

BufferChannel::BufferChannel( Time *tick, int s_size )
  : Channel( tick ), size( s_size ), occupancy( 0 ), contents()
{}

void BufferChannel::send( Packet pack )
{
  if ( occupancy + pack.length <= size ) { /* tail drop behavior */
    contents.push_back( pack );
    occupancy += pack.length;
  }

  try_drain();
}

void BufferChannel::wakeup( void )
{
  try_drain();
}

void BufferChannel::try_drain( void )
{
  while ( (!contents.empty()) && dest && dest->sendable() ) {
    Packet pack = contents.front();
    contents.pop_front();
    occupancy -= pack.length;
    assert( occupancy >= 0 );
    dest->send( pack );
  }
}
