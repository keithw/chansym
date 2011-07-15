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
#include "extractor.hpp"
#include "isender.hpp"
#include "pawn.hpp"

#include "series.cpp"
#include "ensemble_container.cpp"
#include "isender.cpp"

class TwoTerminalNetwork {
public:
  template <class SenderObject, class ReceiverObject>
  class DemoNet {
    typedef Series< Series<SenderObject, Pinger>,
		    Series< Buffer,
			    Series< Series< Throughput, Screener >,
				    Series< Delay, ReceiverObject > > > > Channel;
  };

  typedef typename DemoNet<Pawn, Collector>::Channel SimulatedChannel;
  typedef ISender<SimulatedChannel> SmartSender;
  typedef typename DemoNet<SmartSender, SignallingCollector>::Channel RealChannel;
  
  class TheExtractor : public Extractor<SimulatedChannel>
  {
  public:
    Collector & get_collector( SimulatedChannel &ch )
    {
      return ch.get_second().get_second().get_second().get_second();
    }

    Pawn & get_pawn( SimulatedChannel &ch )
    {
      return ch.get_first().get_first();
    }

    Collector & get_collector( SmartSender *ch )
    {
      Channel *top = ch->get_container_channel()->get_container_channel();
      RealChannel *top_rc = dynamic_cast<RealChannel *>( top );
      assert( top_rc );
      return top_rc->get_second().get_second().get_second().get_second();
    }
  };

  class TheWaker : public Waker
  {
  public:
    void wakeup_smart_sender( Channel *ch, double time )
    {
      Channel *top = ch->get_container_channel()->get_container_channel()->get_container_channel()->get_container_channel();
      RealChannel *top_rc = dynamic_cast<RealChannel *>( top );
      assert( top_rc );
      top_rc->get_first().sleep_until( time, 0, 99 );
    }
  };

  TheExtractor extractor;
  TheWaker waker;

  TwoTerminalNetwork() : extractor(), waker() {}
};

int main( void )
{
  srand( 0 );

  TwoTerminalNetwork network;
  EnsembleContainer<typename TwoTerminalNetwork::RealChannel> truth;
  EnsembleContainer<typename TwoTerminalNetwork::SimulatedChannel> prior;

  truth.set_forking( false );

  prior.add( series( series( Pawn(), Pinger( 3.14159, -1 ) ),
		     series( Buffer( 96000 ),
			     series( series( Throughput( 12000 ), Screener( 0 ) ),
				     series( Delay( 0 ), Collector() ) ) ) ) );

  prior.normalize();

  truth.add( series( series( TwoTerminalNetwork::SmartSender( prior, &network.extractor ), Pinger( 3.14159, -1 ) ),
		     series( Buffer( 96000 ),
			     series( series( Throughput( 12000 ), Screener( 0 ) ),
				     series( Delay( 0 ), SignallingCollector( &network.waker ) ) ) ) ) );

  truth.normalize();

  while ( truth.tick() && (truth.time() < 100) ) {}

  return 0;
}
