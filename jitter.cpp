#include <boost/functional/hash.hpp>
#include <sstream>

#include "jitter.hpp"
#include "container.hpp"

Jitter::Jitter( double s_delay, double s_delay_probability )
  : Channel(), delay( s_delay ), delay_probability( s_delay_probability), contents()
{}

void Jitter::send( Packet pack )
{
  ForkState *x = new ForkState( pack );
  assert( !forking );
  container->fork( addr, 1 - delay_probability, x );
}

void Jitter::after_fork( bool is_other, ForkState x )
{
  if ( x.pack == DummyPacket ) return;

  if ( !is_other ) {
    container->receive( addr, x.pack );
  } else {
    double delivery_time = container->time() + delay;
    contents.push_back( ScheduledPacket( delivery_time, x.pack ) );
    container->sleep_until( delivery_time, addr );
  }

  forking = false;
}

void Jitter::wakeup( void )
{
  assert( !forking );

  ScheduledPacket pack = contents.front();
  contents.pop_front();
  assert( container->time() == pack.delivery_time );
  container->receive( addr, pack.packet );
}

string Jitter::identify( void )
{
  ostringstream response;
  response << "Jitter( ";
  response << delay;
  response << ", ";
  response << delay_probability;
  response << " )";

  return response.str();
}

size_t hash_value( Jitter const & ch )
{
  size_t seed = 0;
  boost::hash_combine( seed, ch.delay );
  boost::hash_combine( seed, ch.delay_probability );
  boost::hash_combine( seed, ch.contents );
  return seed;
}
