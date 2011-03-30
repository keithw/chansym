#include <iostream>
#include <assert.h>

#include "channel.hpp"

void Channel::send( string payload )
{
  double delivery_time = time->now() + delay;
  contents.push_back( Packet( delivery_time, payload ) );
  time->sleep_until( Event( delivery_time, this ) );
}

void Channel::wakeup( void )
{
  while ( !contents.empty() ) {
    Packet pack = contents.front();
    if ( time->now() >= pack.delivery_time ) {
      assert( time->now() - pack.delivery_time < 1e-10 );
      cout << time->now() << "\t" << pack.payload;
      contents.pop_front();
    } else {
      break;
    }
  }
}
