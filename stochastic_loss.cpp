#include "stochastic_loss.hpp"
#include "container.hpp"

#include <stdio.h>

StochasticLoss::StochasticLoss( double s_prob )
  : Channel(), loss_probability( s_prob )
{}

void StochasticLoss::send( Packet pack )
{
  StochasticLoss *other = new StochasticLoss( loss_probability );

  fprintf( stderr, "about to fork, container = %p\n", (void*)container );
  ForkState *x = new ForkState( pack );
  container->fork( addr, 1 - loss_probability, other, x );
}

void StochasticLoss::after_fork_behavior( bool is_other, ForkState x )
{
  if ( !is_other ) {
    fprintf( stderr, "about to receive, container = %p\n", (void*)container );
    container->receive( addr, x.pack );
  }
}
