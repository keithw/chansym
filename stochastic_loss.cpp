#include <sstream>

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

  if ( !is_other ) {
    container->receive( addr, x.pack );
  }

  forking = false;
}

string StochasticLoss::identify( void )
{
  ostringstream response;

  response << "StochasticLoss( ";
  response << loss_probability;
  response << " )";

  return response.str();
}
