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

#include "series.cpp"
#include "ensemble_container.cpp"
#include "isender.cpp"
#include "diverter.cpp"

class TwoTerminalNetwork {
public:
  template <class SenderObject, class ReceiverObject>
  class DemoNet {
    typedef Series< Series<SenderObject,
			   Pinger>,
		    Series< Buffer,
			    Series< Series< Throughput, StochasticLoss >,
				    Diverter< ReceiverObject,
					      Collector > > > > Channel;
  };

  typedef typename DemoNet<Pawn, Collector>::Channel SimulatedChannel;
  typedef ISender<SimulatedChannel> SmartSender;
  typedef typename DemoNet<SmartSender, SignallingCollector>::Channel RealChannel;
  
  template <class ChannelType, class SenderObject, class ReceiverObject>
  class Navigator
  {
  public:
    static ReceiverObject & get_collector( ChannelType *ch )
    {
      return ch->get_second().get_second().get_second().get_first();
    }

    static Collector & get_cross_traffic( ChannelType *ch )
    {
      return ch->get_second().get_second().get_second().get_second();
    }

    static SenderObject & get_sender( ChannelType *ch )
    {
      return ch->get_first().get_first();
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
      Channel *top = ch->get_container_channel()->get_container_channel()->get_container_channel()->get_container_channel();
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
    void wakeup_smart_sender( SignallingCollector *ch, double time ) { real.get_root( ch )->get_first().sleep_until( time, 0, 99 ); }
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
  for ( int bufsize = 48000; bufsize <= 120000; bufsize += 12000 ) {
    for ( int linkspeed = 10000; linkspeed <= 14000; linkspeed += 2000 ) {
      for ( double lossrate = 0; lossrate <= 0.4; lossrate += 0.2 ) {
	for ( double other_sender_speed = linkspeed * 0.2; other_sender_speed <= linkspeed * 0.8; other_sender_speed += linkspeed * 0.1 ) {
	  prior.add( series( series( Pawn(),
				     Pinger( 12000.0 / other_sender_speed, -1 ) ),
			     series( Buffer( bufsize ),
				     series( Throughput( linkspeed ),
					     diverter( series( StochasticLoss( lossrate ),
							       Collector() ),
						       Collector() ) ) ) ) );
	}
      }
    }
  }
  */

  for ( int bufsize = 12000; bufsize <= 120000; bufsize += 12000 ) {
    for ( int linkspeed = 2000; linkspeed <= 20000; linkspeed += 2000 ) {
      for ( double lossrate = 0; lossrate <= 0.4; lossrate += 0.1 ) {
	for ( double other_sender_speed = linkspeed * 0.2; other_sender_speed <= linkspeed * 0.8; other_sender_speed += linkspeed * 0.1 ) {
	  prior.add( series( series( Pawn(),
				     Pinger( 12000.0 / other_sender_speed, -1 ) ),
			     series( Buffer( bufsize ),
				     series( series( Throughput( linkspeed ),
						     StochasticLoss( lossrate ) ),
					     diverter( Collector(),
						       Collector() ) ) ) ) );
	}
      }
    }
  }  

  prior.normalize();

  truth.add( series( series( TwoTerminalNetwork::SmartSender( prior, &network.extractor ),
			     Pinger( 12000 / (12000 * 0.4), -1 ) ),
		     series( Buffer( 96000 ),
			     series( series( Throughput( 12000 ),
					     StochasticLoss( 0.2 ) ),
				     diverter( SignallingCollector( &network.waker ),
					       Collector() ) ) ) ) );

  truth.normalize();

  truth.set_printing( true );

  while ( truth.tick() && (truth.time() < 1000) ) {
    /*
    cout << "===" << endl;
    cout << "True channel at time " << truth.time() << ":" << endl;
    cout << truth.identify();
    */
  }

  return 0;
}
