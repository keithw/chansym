#include <boost/functional/hash.hpp>
#include <sstream>

#include "intermittent.hpp"
#include "container.hpp"

Intermittent::Intermittent( double s_probability, double s_interval )
  : Channel(), working( true ), probability( s_probability ), interval( s_interval )
{}

void Intermittent::init( void )
{
  container->sleep_until( 0, addr, -1 ); /* sort first */
}

void Intermittent::send( Packet pack )
{
  if ( working ) {
    container->receive( addr, pack );
  }
}

void Intermittent::after_fork( bool is_other, ForkState x )
{
  if ( is_other && x.flip ) {
    working = !working;
  }

  forking = false;
}

void Intermittent::wakeup( void )
{
  assert( !forking );
  ForkState *x = new ForkState( true );
  container->sleep_until( container->time() + interval, addr );
  container->fork( addr, 1 - probability, x );
}

string Intermittent::identify( void ) const
{
  ostringstream response;
  response << "Intermittent( ";
  response << (working ? "working" : "blocked");
  response << ", ";
  response << interval;
  response << " )";
  
  return response.str();
}

size_t hash_value( Intermittent const & ch )
{
  size_t seed = 0;
  boost::hash_combine( seed, ch.working );
  boost::hash_combine( seed, ch.probability );
  boost::hash_combine( seed, ch.interval );
  return seed;
}

