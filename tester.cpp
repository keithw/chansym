#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

#include "ensemble_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"
#include "buffer.hpp"
#include "throughput.hpp"
#include "delay.hpp"
#include "stochastic_loss.hpp"
#include "collector.hpp"
#include "screener.hpp"

#include "series.cpp"
#include "ensemble_container.cpp"

int main( void )
{
  srand( 0 );

  EnsembleContainer< Series< Series<Pinger, Pinger>,
			     Series< Buffer,
				     Series< Series< Throughput, StochasticLoss >,
					     Series< Screener, Collector > > > > >
    prior, truth;

  truth.set_forking( false );

  truth.add( series( series( Pinger( 1 ), Pinger( 0.6, -1 ) ),
		     series( Buffer( 96000 ),
			     series( series( Throughput( 6000 ), StochasticLoss( 0.2 ) ),
				     series( Screener( 0 ), Collector() ) ) ) ) );

  for ( double interval = 0.1; interval <= 1.5; interval += 0.1 ) {
    for ( double rate = 0; rate <= 1; rate += 0.2 ) {
      for ( int bufsize = 24000; bufsize <= 120000; bufsize += 24000 ) {
	for ( int throughput = 2000; throughput <= 10000; throughput += 2000 ) {
	  for ( int initpackets = 0; initpackets * 12000 <= bufsize; initpackets++ ) {
	    prior.add( series( series( Pinger( 1 ), Pinger( interval, -1 ) ),
			       series( Buffer( bufsize, initpackets, 12000 ),
				       series( series( Throughput( throughput ), StochasticLoss( rate ) ),
					       series( Screener( 0 ), Collector() ) ) ) ) );
	  }
	}
      }
    }
  }

  fprintf( stderr, "Starting calculation with %d channels...\n", prior.size() );

  while ( truth.time() < 10000 ) {
    /* Advance by smallest timeslice */
    double next_time = truth.next_time() < prior.next_time() ? truth.next_time() : prior.next_time();
    while ( truth.next_time() == next_time ) {
      truth.tick();
    }
    while ( prior.next_time() == next_time ) {
      prior.tick();
    }

    printf( "Time: %f\n", next_time );

    /* Kill mismatches */
    for ( unsigned int i = 0; i < prior.size(); i++ ) {
      if ( prior.get_channel( i ).channel.get_second().get_second().get_second().get_second().get_packets()
	   != truth.get_channel( 0 ).channel.get_second().get_second().get_second().get_second().get_packets() ) {
	prior.erase( i );
      }

      prior.get_channel( i ).channel.get_second().get_second().get_second().get_second().reset();
    }

    prior.prune( 1000 );
    prior.normalize();

    /* reset truth collector */
    truth.get_channel( 0 ).channel.get_second().get_second().get_second().get_second().reset();

    prior.combine();

    cout << prior.identify();
  }

  return 0;
}
