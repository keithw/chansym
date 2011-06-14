#include "delay.hpp"
#include "container.hpp"

Delay::Delay( double s_delay )
  : Channel(), delay( s_delay ), contents()
{}

void Delay::send( Packet pack )
{
  double delivery_time = container->time() + delay;
  contents.push_back( ScheduledPacket( delivery_time, pack ) );
  container->sleep_until( delivery_time, addr );
}

void Delay::wakeup( void )
{
  ScheduledPacket pack = contents.front();
  contents.pop_front();
  assert( container->time() == pack.delivery_time );
  container->receive( addr, pack.packet );
}
