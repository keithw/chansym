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

  typedef Series< Series<Pawn, Buffer>, Series<Throughput, Collector> > EncapsChannelType;
  typedef Series< Series<ISender<EncapsChannelType>, Buffer>, Series<Throughput, Collector> > RealChannelType;

  class ExtractThis : public Extractor<EncapsChannelType> {
    Collector & get_collector( EncapsChannelType &ch ) {
      return ch.get_second().get_second();
    }

    Pawn & get_pawn( EncapsChannelType & ch ) {
      return ch.get_first().get_first();
    }
  };

  ExtractThis extractor;

  EnsembleContainer<RealChannelType> truth;
  EnsembleContainer<EncapsChannelType> prior;

  truth.set_forking( false );

  for ( int bufsize = 12000; bufsize <= 120000; bufsize += 12000 ) {
    for ( int throughput = 1000; throughput <= 20000; throughput += 1000 ) {
      for ( int initpackets = 0; initpackets * 12000 <= bufsize; initpackets++ ) {
	prior.add( series( series( Pawn(),
				   Buffer( bufsize, initpackets, 12000 ) ),
			   series( Throughput( throughput ),
				   Collector() ) ) );
      }
    }
  }

  prior.normalize();

  truth.add( series( series( ISender<EncapsChannelType>( prior, &extractor ),
			     Buffer( 96000 ) ),
		     series( Throughput( 6000 ),
			     Collector() ) ) );

  truth.normalize();

  truth.get_channel( 0 ).channel.get_first().get_first().set_collector( truth.get_channel( 0 ).channel.get_second().get_second() );

  while ( truth.tick() && (truth.time() < 10000) ) {}

  return 0;
}
