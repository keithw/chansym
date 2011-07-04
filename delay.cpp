#include <boost/functional/hash.hpp>
#include <sstream>

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

string Delay::identify( void )
{
  ostringstream response;
  response << "Delay( ";
  response << delay;
  response << " )";

  return response.str();
}

size_t hash_value( Delay const & ch )
{
  size_t seed = 0;
  boost::hash_combine( seed, ch.delay );
  boost::hash_combine( seed, ch.contents );
  return seed;
}
