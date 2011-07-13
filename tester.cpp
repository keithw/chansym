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
#include "isender.hpp"
#include "extractor.hpp"
#include "pawn.hpp"

#include "series.cpp"
#include "ensemble_container.cpp"
#include "isender.cpp"

int main( void )
{
  srand( 0 );

  typedef Series< Series<Pawn, Pinger>,
		  Series< Buffer,
			  Series< Series< Throughput, Screener >,
				  Series< StochasticLoss, Collector > > > > EncapsChannelType;

  typedef Series< Series<ISender<EncapsChannelType>, Pinger>,
		  Series< Buffer,
			  Series< Series< Throughput, Screener >,
				  Series< StochasticLoss, Collector > > > > RealChannelType;

  class ExtractThis : public Extractor<EncapsChannelType> {
    Collector & get_collector( EncapsChannelType &ch ) {
      return ch.get_second().get_second().get_second().get_second();
    }

    Pawn & get_pawn( EncapsChannelType & ch ) {
      return ch.get_first().get_first();
    }
  };

  ExtractThis extractor;

  EnsembleContainer<RealChannelType> truth;
  EnsembleContainer<EncapsChannelType> prior;

  truth.set_forking( false );

  truth.set_printing( true );

  /*
  for ( double interval = 0.1; interval <= 1.5; interval += 0.1 ) {
    for ( double rate = 0; rate <= 1; rate += 0.2 ) {
      for ( int bufsize = 24000; bufsize <= 120000; bufsize += 24000 ) {
	for ( int throughput = 2000; throughput <= 10000; throughput += 2000 ) {
	  for ( int initpackets = 0; initpackets * 12000 <= bufsize; initpackets++ ) {
	    prior.add( series( series( Pawn(), Pinger( interval, -1 ) ),
			       series( Buffer( bufsize, initpackets, 12000 ),
				       series( series( Throughput( throughput ), Screener( 0 ) ),
					       series( StochasticLoss( rate ), Collector() ) ) ) ) );
	  }
	}
      }
    }
  }
  */

  prior.add( series( series( Pawn(), Pinger( 0.6, -1 ) ),
		     series( Buffer( 96000 ),
			     series( series( Throughput( 6000 ), Screener( 0 ) ),
				     series( StochasticLoss( 0.2 ), Collector() ) ) ) ) );

  prior.normalize();

  truth.add( series( series( ISender<EncapsChannelType>( prior, &extractor ), Pinger( 0.6, -1 ) ),
		     series( Buffer( 96000 ),
			     series( series( Throughput( 6000 ), Screener( 0 ) ),
				     series( StochasticLoss( 0.2 ), Collector() ) ) ) ) );

  truth.normalize();

  truth.get_channel( 0 ).channel.get_first().get_first().set_collector( truth.get_channel( 0 ).channel.get_second().get_second().get_second().get_second() );

  while ( truth.tick() && (truth.time() < 10000) ) {}

  return 0;
}
