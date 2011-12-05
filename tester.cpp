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
#include "utility.hpp"
#include "intermittent.hpp"
#include "squarewave.hpp"
#include "timequantize.hpp"

#include "series.cpp"
#include "ensemble_container.cpp"
#include "isender.cpp"
#include "diverter.cpp"
#include "value_iterator.cpp"

class TwoTerminalNetwork {
public:
  template <class SenderObject, class ReceiverObject, class BreakageObject>
  class DemoNet {
    typedef Series< Series<Series< Pinger, BreakageObject >,
			   SenderObject>,
		    Series< Buffer,
			    Series< Series< Throughput, StochasticLoss >,
				    Diverter< Series< TimeQuantize, ReceiverObject >,
					      Collector > > > > Channel;
  };

  typedef typename DemoNet<Pawn, Collector, Intermittent>::Channel SimulatedChannel;
  typedef ISender<SimulatedChannel> SmartSender;
  typedef typename DemoNet<SmartSender, SignallingCollector, SquareWave>::Channel RealChannel;
  
  template <class ChannelType, class SenderObject, class ReceiverObject>
  class Navigator
  {
  public:
    static ReceiverObject & get_collector( ChannelType *ch )
    {
      return ch->get_second().get_second().get_second().get_first().get_second();
    }

    static Collector & get_cross_traffic( ChannelType *ch )
    {
      return ch->get_second().get_second().get_second().get_second();
    }

    static SenderObject & get_sender( ChannelType *ch )
    {
      return ch->get_first().get_second();
    }

    static ChannelType * get_root( SenderObject *ch )
    {
      Channel *top = ch->get_container_channel()->get_container_channel();
      ChannelType *top_rc = dynamic_cast<ChannelType *>( top );
      assert( top_rc );
      return top_rc;
    }

    static ChannelType * get_root( ReceiverObject *ch )
    {
      Channel *top = ch->get_container_channel()->get_container_channel()->get_container_channel()->get_container_channel()->get_container_channel();
      ChannelType *top_rc = dynamic_cast<RealChannel *>( top );
      assert( top_rc );
      return top_rc;
    }

    static void reset( ChannelType *ch )
    {
      get_collector( ch ).reset();
      get_cross_traffic( ch ).reset();
    }
  };

public:
  class TheExtractor : public Extractor<SimulatedChannel>
  {
  private:
    Navigator<SimulatedChannel, Pawn, Collector> sim;
    Navigator<RealChannel, SmartSender, SignallingCollector> real;

  public:
    TheExtractor() : sim(), real() {}

    /* Let the true sender find its collector and cross traffic */
    SignallingCollector & get_collector( SmartSender *ch ) { return real.get_collector( real.get_root( ch ) ); }
    Collector & get_cross_traffic( SmartSender *ch ) { return real.get_cross_traffic( real.get_root( ch ) ); }

    /* Let him reset all collectors */
    void reset( SmartSender *ch ) { return real.reset( real.get_root( ch ) ); }

    /* Let an omniscient find the simulated collector, pawn and cross traffic */
    Collector & get_collector( SimulatedChannel &ch ) { return sim.get_collector( &ch ); }
    Collector & get_cross_traffic( SimulatedChannel &ch ) { return sim.get_cross_traffic( &ch ); }
    Pawn & get_pawn( SimulatedChannel &ch ) { return sim.get_sender( &ch ); }

    /* Let him reset all collectors */
    void reset( SimulatedChannel &ch ) { return sim.reset( &ch ); }

    /* Let us find the true collector and cross traffic */
    Collector & get_collector( RealChannel &ch ) { return real.get_collector( &ch ); }
    Collector & get_cross_traffic( RealChannel &ch ) { return real.get_cross_traffic( &ch ); }
  };

  class TheWaker : public Waker
  {
  private:
    Navigator<RealChannel, SmartSender, SignallingCollector> real;

  public:
    TheWaker() : real() {}
    void wakeup_smart_sender( SignallingCollector *ch, double time ) { real.get_root( ch )->get_first().sleep_until( time, 1, 99 ); }
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

  truth.set_follow_all_forks( false );

  /*
  for ( double link_portion = 0.1; link_portion <= 0.5; link_portion += 0.05 ) {
    for ( int bufsize = 12000*10; bufsize <= 12000*50; bufsize += 12000*10 ) {
      for ( int init = 0; init * 12000 <= bufsize; init += 40 ) {
        for ( int linkspeed = 12000*6; linkspeed <= 12000*20; linkspeed += 12000*2 ) {
	  for ( double lossrate = 0.0; lossrate <= 0.6; lossrate += 0.05 ) {
	    prior.add( series( series( series( Pinger( 12000.0 / (linkspeed * link_portion), 1, true ), Intermittent( .0034, .5 ) ),
				       Pawn() ),
			       series( Buffer( bufsize, init, 12000 ),
				       series( series( Throughput( linkspeed ), StochasticLoss( lossrate ) ),
					       diverter( series( TimeQuantize( 0.1 ), Collector() ),
							 Collector() ) ) ) ) );
	  }
        }
      }
    }
  }

  prior.normalize();
  */

  double my_linkspeed = 12000*10;
  double my_bufsize = 12000*3;
  double my_lossrate = 0.2;
  double my_linkportion = 0.5;

  prior.add( series( series( series( Pinger( 12000.0 / (my_linkspeed * my_linkportion), 1, true ), Intermittent( .00069, .1 ) ),
			     Pawn() ),
		     series( Buffer( my_bufsize ),
			     series( series( Throughput( my_linkspeed ), StochasticLoss( my_lossrate ) ),
				     diverter( series( TimeQuantize( 0.1 ), Collector() ),
					       Collector() ) ) ) ) );
  prior.normalize();

  truth.add( series( series( series( Pinger( 12000 / (my_linkspeed * my_linkportion), 1, true ), SquareWave( 200 ) ),
                             TwoTerminalNetwork::SmartSender( prior, &network.extractor ) ),
                     series( Buffer( my_bufsize ),
                             series( series( Throughput( my_linkspeed ), StochasticLoss( my_lossrate ) ),
                                     diverter( series( TimeQuantize( 0.1 ), SignallingCollector( &network.waker ) ),
                                               Collector() ) ) ) ) );

  truth.normalize();

  truth.set_printing( true );

  printf( "Starting with %d channels...\n", prior.size() );
  cout << prior.identify();
  fflush( NULL );

  while ( truth.tick() && (truth.time() < 350) ) {
    cout << "===" << endl;
    cout << "True channel at time " << truth.time() << ":" << endl;
    cout << truth.identify();
  }

  return 0;
}
