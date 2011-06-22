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
  srand( 0 );

  EnsembleContainer< Series< Series<Pinger, Buffer>,
			     Series< Series< Throughput, StochasticLoss >,
				     Collector > > >
    prior, truth;

  truth.set_forking( false );

  truth.add( series( series( Pinger( 1 ), Buffer( 24000 ) ),
		     series( series( Throughput( 6000 ), StochasticLoss( 0.5 ) ),
			     Collector() ) ) );

  prior.add( series( series( Pinger( 1 ), Buffer( 24000 ) ),
		     series( series( Throughput( 6000 ), StochasticLoss( 0.5 ) ),
			     Collector() ) ) );

  prior.add( series( series( Pinger( 1 ), Buffer( 24000 ) ),
		     series( series( Throughput( 6000 ), StochasticLoss( 0.45 ) ),
			     Collector() ) ) );


  while ( 1 ) {
    /* Advance by smallest timeslice */
    double next_time = truth.next_time();
    while ( truth.next_time() == next_time ) {
      truth.tick();
    }
    while ( prior.next_time() == next_time ) {
      prior.tick();
    }
    assert( truth.time() == prior.time() );
    printf( "Time: %f\n", truth.time() );

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
  }

  return 0;
}
