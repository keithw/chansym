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
#include "jitter.hpp"

#include "series.cpp"
#include "ensemble_container.cpp"

int main( void )
{
  srand( 0 );

  EnsembleContainer< Series< Series< Series<Pinger, Pinger>,
				     Series<Buffer, Throughput> >,
			     Series< Series< Series<Screener, Jitter>,
					     Series<Delay, StochasticLoss> >,
				     Collector > > >
    prior, truth;

  truth.set_forking( false );

  truth.add( series( series( series( Pinger( 1 ), Pinger( 0.6, -1 ) ),
			     series( Buffer( 96000 ), Throughput( 6000 ) ) ),
		     series( series( series( Screener( 0 ), Jitter( 2, 0.1 ) ),
				     series( Delay( 2 ), StochasticLoss( 0.2 ) ) ),
			     Collector() ) ) );

  for ( double other_interval = 0.2; other_interval <= 1.4; other_interval += 0.2 ) {
    for ( int bufsize = 24000; bufsize <= 120000; bufsize += 24000 ) {
      for ( int initpackets = 0; initpackets * 12000 <= bufsize; initpackets++ ) {
	for ( int throughput = 2000; throughput <= 10000; throughput += 2000 ) {
	  for ( double delay = 1; delay <= 3; delay += 0.25 ) {
	    for ( double delayp = 0; delayp <= 1; delayp += 0.1 ) {
	      for ( double lossrate = 0; lossrate <= 0.6; lossrate += 0.2 ) {
		prior.add( series( series( series( Pinger( 1 ), Pinger( other_interval, -1 ) ),
					   series( Buffer( bufsize, initpackets, 12000 ), Throughput( throughput ) ) ),
				   series( series( series( Screener( 0 ), Jitter( delay, delayp ) ),
						   series( Delay( 2 ), StochasticLoss( lossrate ) ) ),
					   Collector() ) ) );
	      }
	    }
	  }
	}
      }
    }
  }

  truth.normalize();
  prior.normalize();

  fprintf( stderr, "Starting calculation with %d channels...\n", prior.size() );

  int ticknum = 0;
  unsigned int smallestsize = prior.size();

  while ( truth.time() < 10000 ) {
    /* Advance by smallest timeslice */
    double next_time = truth.next_time() < prior.next_time() ? truth.next_time() : prior.next_time();
    while ( truth.next_time() == next_time ) {
      truth.tick();
    }
    while ( prior.next_time() == next_time ) {
      prior.tick();
    }

    printf( "Time: %f (channels: %d)\n", next_time, prior.size() );

    /* Kill mismatches */
    for ( unsigned int i = 0; i < prior.size(); i++ ) {
      if ( prior.get_channel( i ).channel.get_second().get_second().get_packets()
	   != truth.get_channel( 0 ).channel.get_second().get_second().get_packets() ) {
	prior.erase( i );
      }

      prior.get_channel( i ).channel.get_second().get_second().reset();
    }

    /* reset truth collector */
    truth.get_channel( 0 ).channel.get_second().get_second().reset();

    prior.prune( 1000 );

    prior.normalize();

    if ( (prior.size() >= 2 * smallestsize) || (prior.get_erased_count() * 2 >= (int)prior.size()) ) {
      prior.combine();
      smallestsize = prior.size();
    }

    if ( prior.size() <= 32 )
      cout << prior.identify();

    ticknum++;
  }

  return 0;
}
