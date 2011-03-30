#include <iostream>
#include <assert.h>

#include "delay_channel.hpp"

DelayChannel::DelayChannel( Time *tick, double s_delay )
  : Channel( tick ), delay( s_delay ), contents()
{}

void DelayChannel::send( Packet pack )
{
  double delivery_time = time->now() + delay;
  contents.push_back( ScheduledPacket( delivery_time, pack ) );
  time->sleep_until( Event( delivery_time, this ) );
}

void DelayChannel::wakeup( void )
{
  while ( !contents.empty() ) {
    ScheduledPacket pack = contents.front();
    if ( time->now() >= pack.delivery_time ) {
      assert( time->now() - pack.delivery_time < 1e-10 );
      cout << time->now() << "\t" << pack.packet.src << " " << pack.packet.length << endl;
      contents.pop_front();
    } else {
      break;
    }
  }
}
