#include "throughput.hpp"
#include "container.hpp"

Throughput::Throughput( double s_throughput )
  : Channel(), throughput( s_throughput ), next_free_time( -1 ), stash( -1, -1, -1, -1 )
{}

void Throughput::send( Packet pack )
{
  assert( is_free() ); /* can't send if busy */

  stash = pack;

  next_free_time = container->time() + (double) pack.length / throughput;
  container->sleep_until( next_free_time, addr );
}

void Throughput::wakeup( void )
{
  assert( next_free_time == container->time() );
  next_free_time = -1;
  container->receive( addr, stash );
  container->signal_sendable( addr );
}

bool Throughput::is_busy( void )
{
  return next_free_time > container->time();
}

bool Throughput::is_free( void )
{
  return !is_busy();
}
