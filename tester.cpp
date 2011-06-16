#include <stdio.h>

#include "ensemble_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"
#include "buffer.hpp"
#include "throughput.hpp"
#include "delay.hpp"
#include "stochastic_loss.hpp"

#include "series.cpp"
#include "ensemble_container.cpp"

int main( void )
{
  EnsembleContainer< Series< Series<Pinger, StochasticLoss>, Printer > >
    overall( series( series( Pinger( 1 ), StochasticLoss( 0.5 ) ),
		     Printer() ) );

  while ( overall.tick() && (overall.time() < 500) ) {}

  return 0;
}
