#include <boost/functional/hash.hpp>

#include <sstream>

#include "pinger.hpp"
#include "container.hpp"

Pinger::Pinger( double s_increment, int s_id )
  : next_ping_time( -1 ), increment( s_increment ), counter( 0 ), id( s_id )
{}

void Pinger::init( void )
{
  next_ping_time = container->time() + increment;
  container->sleep_until( next_ping_time, addr );
}

void Pinger::wakeup( void )
{
  assert( container->time() == next_ping_time );

  container->receive( addr, Packet( 12000, id, counter++, container->time() ) );

  next_ping_time += increment;
  container->sleep_until( next_ping_time, addr );
}

string Pinger::identify( void )
{
  ostringstream response;
  response << "Pinger( ";
  response << increment;
  response << " )";

  return response.str();
}

size_t hash_value( Pinger const & ch )
{
  size_t seed = 0;
  boost::hash_combine( seed, ch.next_ping_time );
  boost::hash_combine( seed, ch.increment );
  boost::hash_combine( seed, ch.counter );
  boost::hash_combine( seed, ch.id );

  return seed;
}
