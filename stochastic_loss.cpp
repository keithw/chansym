#include "stochastic_loss.hpp"
#include "container.hpp"

#include <stdio.h>

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
  forking = false;

  assert( container );

  if ( !is_other ) {
    container->receive( addr, x.pack );
  }
}
