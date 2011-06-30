#include <sstream>
#include <boost/functional/hash.hpp>

#include "throughput.hpp"
#include "container.hpp"

Throughput::Throughput( double s_throughput )
  : Channel(), throughput( s_throughput ), next_free_time( -1 ), stash( -1, -1, -1, -1 ), busy( false )
{}

void Throughput::send( Packet pack )
{
  assert( is_free() ); /* can't send if busy */
  busy = true;

  stash = pack;

  next_free_time = container->time() + (double) pack.length / throughput;
  container->sleep_until( next_free_time, addr );
}

void Throughput::wakeup( void )
{
  assert( next_free_time == container->time() );
  next_free_time = -1;
  container->receive( addr, stash );
  stash = Packet( -1, -1, -1, -1 ); /* needed for compaction */
  busy = false;
  container->signal_sendable( addr );
}

string Throughput::identify( void )
{
  ostringstream response;
  response << "Throughput( ";
  response << throughput;
  response << " )";

  return response.str();
}

size_t hash_value( Throughput const & ch )
{
  size_t seed = 0;

  boost::hash_combine( seed, ch.throughput );
  boost::hash_combine( seed, ch.next_free_time );
  boost::hash_combine( seed, ch.stash );
  boost::hash_combine( seed, ch.busy );  

  return seed;
}
