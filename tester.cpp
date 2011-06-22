#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
#include "simple_container.cpp"

int main( void )
{
  srand( time( NULL ) );

  SimpleContainer< Series< Series<Pinger, Buffer>,
    Series< Throughput, StochasticLoss > > >
    overall( series( series( Pinger( 0.006 ), Buffer( 1000000 ) ),
		     series( Throughput( 18000 ), StochasticLoss( 0.05 ) ) ) );

  while ( overall.tick() && (overall.time() < 3000) ) {}

  return 0;
}
