#include <boost/functional/hash.hpp>
#include <iostream>

#include "isender.hpp"
#include "extractor.hpp"
#include "pawn.hpp"
#include "close.hpp"
#include "utility.hpp"

#include "embeddable_ensemble.cpp"
#include "utility_ensemble.cpp"

#define        MIN(a,b) (((a)<(b))?(a):(b))

template <class ChannelType>
ISender<ChannelType>::ISender( EnsembleContainer<ChannelType> s_prior,
			       Extractor<ChannelType> *s_extractor )
  : prior( s_prior ),
    extractor( s_extractor ),
    latest_time( -1 ),
    next_send_time( -1000 ), counter( 0 ),
    id( 0 ), smallestsize( prior.size() )
{}

template <class ChannelType>
ISender<ChannelType>::ISender( const ISender<ChannelType> &x )
  : Channel( x ),
    prior( x.prior ), extractor( x.extractor ),
    latest_time( x.latest_time ),
    next_send_time( x.next_send_time ), counter( x.counter ),
    id( x.id ), smallestsize( x.smallestsize )
{}

template <class ChannelType>
ISender<ChannelType> & ISender<ChannelType>::operator=( const ISender<ChannelType> &x )
{
  Channel::operator=( x );

  prior = x.prior;
  extractor = x.extractor;
  latest_time = x.latest_time;

  next_send_time = x.next_send_time;
  counter = x.counter;

  id = x.id;
  smallestsize = x.smallestsize;

  return *this;
}

template <class ChannelType>
void ISender<ChannelType>::init( void )
{
  container->sleep_until( container->time(), addr, 99 );
}

template <class ChannelType>
void ISender<ChannelType>::wakeup( void )
{
  double current_time = container->time();

  /* find true collector contents */
  Collector *collector = &extractor->get_collector( this );
  vector<ScheduledPacket> true_received = collector->get_packets();

  /* Do we need to run? */
  if ( (latest_time == current_time)
       && (true_received.empty())
       && (current_time != next_send_time) ) {
    return;
  } else {
    latest_time = current_time;
  }

  /* advance prior to actual time */
  prior.advance_to( current_time );

  if ( current_time == next_send_time ) {
    sendout( Packet( 12000, id, counter++, current_time ) );
    next_send_time = -1000;
  }

  if ( (!true_received.empty()) || (next_send_time < current_time) ) {
    /* reschedule ping if necessary */
    optimal_action();
  }

  if ( prior.live() ) {
    container->sleep_until( prior.next_time(), addr, 99 );
  }

  /* find and kill mismatches */
  for ( unsigned int i = 0; i < prior.size(); i++ ) {
    if ( prior.get_channel( i ).erased ) {
      continue;
    }

    if ( extractor->get_collector( prior.get_channel( i ).channel ).get_packets()
	 != true_received ) {
      prior.erase( i );
    }

    extractor->reset( prior.get_channel( i ).channel );
  }

  /* reset real collector */
  extractor->reset( this );

  prior.prune( 1000 );

  prior.normalize();

  if ( (prior.size() >= 2 * smallestsize) || (prior.get_erased_count() * 2 >= (int)prior.size()) ) {
    prior.combine();
    smallestsize = prior.size();
  }

  printf( "Time: %f (channels: %d)\n", current_time, prior.size() );
  //  if ( prior.size() <= 32 ) {
    cout << prior.identify();
    //  }
  fflush( NULL );
}

template <class ChannelType>
size_t ISender<ChannelType>::hash( void ) const
{
  boost::hash<EnsembleContainer<ChannelType>> hasher;
  return hasher( prior );
}

template <class ChannelType>
void ISender<ChannelType>::sendout( Packet p )
{
  assert( container->time() == p.send_time );
  printf( "SENDING packet %d at time %f\n",
	  p.id, container->time() );

  /* Really send packet */
  container->receive( addr, p );

  /* Send packet in simulation */
  for ( unsigned int i = 0; i < prior.size(); i++ ) {
    if ( !prior.get_channel( i ).erased ) {
      extractor->get_pawn( prior.get_channel( i ).channel ).send( p );
    }
  }

  prior.execute_fork();
}

class Strategy
{
public:
  double utility;
  double delay;

  Strategy( double s_utility, double s_delay ) : utility( s_utility ), delay( s_delay ) {}
  Strategy( void ) : utility( -100 ), delay( -100 ) {}

  bool operator() ( const Strategy &lhs, const Strategy &rhs ) const
  {
    if ( lhs.utility < rhs.utility ) {
      return 1;
    } else if ( lhs.utility > rhs.utility ) {
      return 0;
    } else {
      return lhs.delay > rhs.delay;
    }
  }
};

template <class ChannelType>
void ISender<ChannelType>::optimal_action( void )
{
  //  const double EARLY_WAKEUP = -100;

  assert( container->time() == prior.time() );

  double base_time = prior.time();

  UtilityEnsemble< EmbeddableEnsemble< ChannelType > > fans( base_time );

  vector<double> delays;
  delays.push_back( -1 );

  //  const double LIMIT = 60;
  const int STEP_LIMIT = 10000;

  delays.push_back( 0 );

  for ( double x = 0.5; x <= 6.0; x += 0.5 ) {
    delays.push_back( x );
  }

  /*
  peekable_priority_queue<Event, deque<Event>, Event> wakeups_copy( prior.get_wakeups() );

  double latest = 0;
  while ( !wakeups_copy.empty() ) {
    double proposal = wakeups_copy.top().time;
    if ( proposal > latest ) {
      delays.push_back( proposal - base_time );
      latest = proposal;
    }
    wakeups_copy.pop();
  }
  */

  //  delays.push_back( 60 );

  //  double last_delay = 60;

  vector<bool> sent_yet;

  priority_queue<Event, deque<Event>, Event> delay_queue;

  EnsembleContainer<ChannelType> prior_stripped( prior );

  for ( unsigned int i = 0; i < prior_stripped.size(); i++ ) {
    /* clear pending packets */
    extractor->reset( prior_stripped.get_channel( i ).channel );
  }

  /* add channels to fan */
  for ( unsigned int i = 0; i < delays.size(); i++ ) {
    fans.add_mature( prior_stripped );
    double the_delay = base_time + delays[ i ];
    fans.get_channel( i ).delay = the_delay;
    if ( the_delay >= base_time ) {
      delay_queue.push( Event( the_delay, i, 0 ) );
      sent_yet.push_back( false );
    } else {
      sent_yet.push_back( true );
    }
  }

  /* initialize utilities */
  for ( unsigned int i = 0; i < fans.size(); i++ ) {
    for ( unsigned int j = 0; j < fans.get_channel( i ).channel.size(); j++ ) {
      fans.get_channel( i ).channel.get_channel( j ).utility = 0;
    }
  }

  int steps = 0;
  while ( 1 ) {
    steps++;

    if ( steps > STEP_LIMIT ) {
      printf( "Error: Iterated %d steps to t=%f but fan has not converged\n",
	      steps, fans.time() );

      printf( "===\n" );
      for ( unsigned int i = 0; i < fans.size(); i++ ) {
	printf( "Overall equality between 0 and %d: %d\n", i,
		fans.get_channel( 0 ).channel == fans.get_channel( i ).channel );

	if ( !(fans.get_channel( 0 ).channel == fans.get_channel( i ).channel) ) {
	  printf( "time[0] = %f, time[%d] = %f\n",
		  fans.get_channel( 0 ).channel.time(), i,
		  fans.get_channel( i ).channel.time() );

	  for ( unsigned int j = 0; j < fans.get_channel( 0 ).channel.size(); j++ ) {
	    printf( "0[%d] == %d[%d]? prob=%d, erased=%d, channel=%d\n",
		    j, i, j,
		    close( fans.get_channel( 0 ).channel.get_channel( j ).probability, fans.get_channel( i ).channel.get_channel( j ).probability ),
		    fans.get_channel( 0 ).channel.get_channel( j ).erased == fans.get_channel( i ).channel.get_channel( j ).erased,
		    fans.get_channel( 0 ).channel.get_channel( j ).channel == fans.get_channel( i ).channel.get_channel( j ).channel );
	  }
	}
      }

      cout << fans.identify();
      exit( 1 );
    }

    if ( delay_queue.empty() && fans.live() ) {
      fans.advance_to( fans.next_time() );
    } else if ( !delay_queue.empty() ) {
      Event next_send = delay_queue.top();
      if ( fans.live() ) {
	fans.advance_to( MIN( next_send.time, fans.next_time() ) );
      } else {
	fans.advance_to( next_send.time );
      }

      if ( fans.time() == next_send.time ) {
	/* send */
	delay_queue.pop();

	assert( !sent_yet[ next_send.addr ] );
	assert( fans.get_channel( next_send.addr ).delay == next_send.time );
	for ( unsigned int i = 0; i < fans.get_channel( next_send.addr ).channel.size(); i++ ) {
	  //	  if ( fans.get_channel( next_send.addr ).channel.get_channel( i ).delay != EARLY_WAKEUP ) {
	    extractor->get_pawn( fans.get_channel( next_send.addr ).channel.get_channel( i ).channel ).send( Packet( 12000, 0, 0, fans.time() ) );
	    //	  }
	}
	sent_yet[ next_send.addr ] = true;
      }
    }

    //    printf( "==> Iterating at time %f\n", fans.time() );

    /* iterate through channels */
    for ( unsigned int i = 0; i < fans.size(); i++ ) {
      for ( unsigned int j = 0; j < fans.get_channel( i ).channel.size(); j++ ) {
	/*
	if ( (!sent_yet[ i ]) && (extractor->get_collector( fans.get_channel( i ).channel.get_channel( j ).channel ).get_packets().size() != 0) ) {
	  fans.get_channel( i ).channel.get_channel( j ).delay = EARLY_WAKEUP; // early wakeup same as non-sending 
	}
      */

	double the_utility = UtilityMetric::utility( base_time, extractor->get_collector( fans.get_channel( i ).channel.get_channel( j ).channel ).get_packets(),
						     extractor->get_cross_traffic( fans.get_channel( i ).channel.get_channel( j ).channel ).get_packets() );
	fans.get_channel( i ).channel.get_channel( j ).utility += the_utility;

	/*
	vector<ScheduledPacket> pks = extractor->get_collector( fans.get_channel( i ).channel.get_channel( j ).channel ).get_packets();

	vector<ScheduledPacket> cpks = extractor->get_cross_traffic( fans.get_channel( i ).channel.get_channel( j ).channel ).get_packets();
	*/

	//	assert( cpks.size() == 0 );

	/*
	for ( unsigned int p = 0; p < pks.size(); p++ ) {
	  assert ( fabs(pks[ p ].delivery_time - fans.time()) < 1e-10 );
	  printf( "Simulating at time %f, arriving at time %f, strategy delay=%f received packet < %d, %d, %f > ==> utility +%f\n",
		  fans.time(), pks[ p ].delivery_time, fans.get_channel( i ).delay - base_time, pks[ p ].packet.src, pks[ p ].packet.id, pks[ p ].packet.send_time,
		  utility( vector<ScheduledPacket>( 1, pks[ p ] ) ) );
	}
	*/

	/*
	if ( the_utility > 0 ) {
	  printf( "UTIL: adding utility %f to delay=%f\n", the_utility, fans.get_channel( i ).delay - base_time );
	}
	*/

	extractor->reset( fans.get_channel( i ).channel.get_channel( j ).channel );
      }

      if ( (steps % 5) == 4 ) {
	fans.get_channel( i ).channel.combine();
      }
    }

    /*
    printf( "===\nIterating at time %f\n", fans.time() );
    cout << fans.identify();
    */

    if ( delay_queue.empty() && fans.converged() ) {
      break;
    }
  }

  /* total up utilities */
  priority_queue< Strategy, deque<Strategy>, Strategy > strategies;

  for ( unsigned int i = 0; i < fans.size(); i++ ) {
    double total_probability = 0;
    double utility = 0;

    EmbeddableEnsemble<ChannelType> &ch = fans.get_channel( i ).channel;

    for ( unsigned int j = 0; j < ch.size(); j++ ) {
      utility += ch.get_channel( j ).utility * ch.get_channel( j ).probability;
      total_probability += ch.get_channel( j ).probability;
    }

    assert( close( total_probability, 1.0 ) );
    strategies.push( Strategy( nearbyint( utility * 10000 ) / 10000.0, fans.get_channel( i ).delay ) );
  }

  next_send_time = strategies.top().delay;

  if ( next_send_time >= base_time ) {
    printf( "Sending solution found after %d steps: wait %f\n", steps, next_send_time - base_time );
    container->sleep_until( next_send_time, addr, 99 );
  } else {
    printf( "Sending solution found after %d steps: wait forever\n", steps );
    //    container->sleep_until( base_time + last_delay, addr, 99 );
  }

  while ( !strategies.empty() ) {
    printf( "At time %f, utility (%f/%f) comes from delay of %f\n",
	    container->time(), strategies.top().utility,
	    fans.time() - base_time,
	    strategies.top().delay - container->time() );
    strategies.pop();
  }
}

