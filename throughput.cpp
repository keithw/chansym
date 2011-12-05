#include <sstream>
#include <boost/functional/hash.hpp>

#include "throughput.hpp"
#include "container.hpp"
#include "close.hpp"

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

string Throughput::identify( void ) const
{
  ostringstream response;
  response << "Throughput( ";
  response << throughput;
  if ( is_busy() ) {
    response << ", free at ";
    response << next_free_time;
  }

  response << ", < ";
  response << stash.src;
  response << ", ";
  response << stash.id;
  response << ", ";
  response << stash.send_time;
  response << " >";

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

void Throughput::quantize_markovize( void )
{
  double time_per_packet = quantize_time( double(PACKET_SIZE) / throughput );
  throughput = double(PACKET_SIZE) / time_per_packet;

  double now = container->time();

  if ( next_free_time != -1 ) {
    next_free_time = quantize_time( next_free_time - now );
  }

  stash.quantize_markovize( now );
}
