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

#include "series.cpp"
#include "ensemble_container.cpp"
#include "simple_container.cpp"

int main( void )
{
  srand( 0 );

  EnsembleContainer< Series< Series<Pinger, Buffer>,
			     Series< Series< Throughput, StochasticLoss >,
				     Collector > > >
    prior, truth;

  truth.set_forking( false );

  truth.add( series( series( Pinger( 1 ), Buffer( 240000 ) ),
		     series( series( Throughput( 6000 ), StochasticLoss( 0.5 ) ),
			     Collector() ) ) );

  for ( double rate = 0; rate <= 1.0; rate += 0.1 ) {
    for ( int bufsize = 120000; bufsize < 480000; bufsize += 10000 ) {
      for ( int throughput = 1000; throughput <= 20000; throughput += 1000 ) {
	prior.add( series( series( Pinger( 1 ), Buffer( bufsize ) ),
			   series( series( Throughput( throughput ), StochasticLoss( rate ) ),
				   Collector() ) ) );
      }
    }
  }

  while ( truth.time() < 100000 ) {
    /* Advance by smallest timeslice */
    double next_time = truth.next_time() < prior.next_time() ? truth.next_time() : prior.next_time();
    //    printf( "Ticking truth through time %f...", next_time );
    while ( truth.next_time() == next_time ) {
      truth.tick();
    }
    //    printf( "done, truth's next time is now %f.\n", truth.next_time() );
    //    printf( "Ticking prior through time %f...", next_time );
    while ( prior.next_time() == next_time ) {
      prior.tick();
    }
    //    printf( "done, prior's next time is now %f.\n", prior.next_time() );

    printf( "Time: %f\n", next_time );

    /* Kill mismatches */
    for ( unsigned int i = 0; i < prior.size(); i++ ) {
      if ( prior.get_channel( i ).channel.get_second().get_second().get_packets()
	   != truth.get_channel( 0 ).channel.get_second().get_second().get_packets() ) {
	prior.erase( i );
      }

      prior.get_channel( i ).channel.get_second().get_second().reset();
    }

    prior.normalize();

    /* reset truth collector */
    truth.get_channel( 0 ).channel.get_second().get_second().reset();

    prior.combine();

    cout << prior.identify();
  }

  return 0;
}
