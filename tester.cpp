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
#include "collector.hpp"

#include "series.cpp"
#include "ensemble_container.cpp"
#include "simple_container.cpp"

int main( void )
{
  srand( time( NULL ) );

  EnsembleContainer< Series< Series<Pinger, Buffer>,
			     Series< Series< Throughput, StochasticLoss >,
				     Collector > > >
  overall( series( series( Pinger( 0.006 ), Buffer( 1000000 ) ),
		   series( series( Throughput( 18000 ), StochasticLoss( 0.05 ) ),
			   Collector() ) ) );

  overall.set_printing( true );
  overall.set_forking( false );

  while ( overall.tick() && (overall.time() < 3000) ) {
    for ( unsigned int i = 0; i < overall.size(); i++ ) {
      overall.get_channel( i ).channel.get_second().get_second().reset();
    }
    overall.combine();
  }

  return 0;
}
