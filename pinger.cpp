#include "pinger.hpp"
#include "container.hpp"

Pinger::Pinger( double s_increment )
  : next_ping_time( -1 ), increment( s_increment ), counter( 0 )
{}

void Pinger::init( void )
{
  assert( container );
  next_ping_time = container->time();
  container->sleep_until( next_ping_time, addr );
}

void Pinger::wakeup( void )
{
  assert( container );
  assert( container->time() == next_ping_time );
  container->receive( addr, Packet( 12000, 0, counter, container->time() ) );
  counter++;
  next_ping_time += increment;
  container->sleep_until( next_ping_time, addr );
}
