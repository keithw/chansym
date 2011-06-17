#include "pinger.hpp"
#include "container.hpp"

Pinger::Pinger( double s_increment )
  : next_ping_time( -1 ), increment( s_increment ), counter( 0 )
{}

void Pinger::init( void )
{
  next_ping_time = container->time();
  container->sleep_until( next_ping_time, addr );
}

void Pinger::wakeup( void )
{
  assert( container->time() == next_ping_time );
  counter++;
  next_ping_time += increment;
  container->sleep_until( next_ping_time, addr );
  container->receive( addr, Packet( 12000, 0, counter - 1, container->time() ) );
}