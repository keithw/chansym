#include <sstream>
#include <boost/functional/hash.hpp>

#include "stochastic_loss.hpp"
#include "container.hpp"

StochasticLoss::StochasticLoss( double s_prob )
  : Channel(), loss_probability( s_prob )
{}

void StochasticLoss::send( Packet pack )
{
  ForkState *x = new ForkState( pack );
  assert( !forking );
  container->fork( addr, 1 - loss_probability, x );
}

void StochasticLoss::after_fork( bool is_other, ForkState x )
{
  assert( container );

  if ( x.pack == DummyPacket ) return;

  if ( !is_other ) {
    container->receive( addr, x.pack );
  }

  forking = false;
}

string StochasticLoss::identify( void ) const
{
  ostringstream response;

  response << "StochasticLoss( ";
  response << loss_probability;
  response << " )";

  return response.str();
}

size_t hash_value( StochasticLoss const & ch )
{
  boost::hash<double> hasher;
  return hasher( ch.loss_probability );
}
