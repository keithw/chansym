#include <boost/functional/hash.hpp>
#include <iostream>

#include "isender.hpp"
#include "extractor.hpp"
#include "pawn.hpp"

#include "embeddable_ensemble.cpp"
#include "utility_ensemble.cpp"

#define        MIN(a,b) (((a)<(b))?(a):(b))

template <class ChannelType>
ISender<ChannelType>::ISender( EnsembleContainer<ChannelType> s_prior,
			       Extractor<ChannelType> *s_extractor )
  : prior( s_prior ),
    extractor( s_extractor ),
    latest_time( -1 ),
    next_send_time( -1 ), counter( 0 ),
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
    next_send_time = -1;
  }

  /* ping if necessary */
  optimal_action();

  container->sleep_until( prior.next_time(), addr, 99 );

  /* find and kill mismatches */
  for ( unsigned int i = 0; i < prior.size(); i++ ) {
    if ( prior.get_channel( i ).erased ) {
      continue;
    }

    if ( extractor->get_collector( prior.get_channel( i ).channel ).get_packets()
	 != true_received ) {
      prior.erase( i );
    }

    extractor->get_collector( prior.get_channel( i ).channel ).reset();
  }

  /* reset real collector */
  collector->reset();

  prior.prune( 1000 );

  prior.normalize();

  if ( (prior.size() >= 2 * smallestsize) || (prior.get_erased_count() * 2 >= (int)prior.size()) ) {
    prior.combine();
    smallestsize = prior.size();
  }

  printf( "Time: %f (channels: %d)\n", current_time, prior.size() );
  if ( prior.size() <= 32 ) {
    cout << prior.identify();
  }
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

static double utility( vector<ScheduledPacket> x )
{
  double util = 0;

  for ( vector<ScheduledPacket>::const_iterator i = x.begin();
	i != x.end();
	i++ ) {
    double delay = i->delivery_time - i->packet.send_time;
    assert( delay >= 0 );
    assert( delay < 10 );

    util += 10 - delay;
  }

  return util;
}

template <class ChannelType>
void ISender<ChannelType>::optimal_action( void )
{
  const double EARLY_WAKEUP = -100;

  assert( container->time() == prior.time() );

  double base_time = prior.time();

  UtilityEnsemble< EmbeddableEnsemble< ChannelType > > fans( base_time );

  vector<double> delays;
  delays.push_back( -1 );

  const double LIMIT = 1;

  for ( double d = 0; d < LIMIT; d += 0.1 ) {
    delays.push_back( d );
  }

  vector<bool> sent_yet;

  priority_queue<Event, deque<Event>, Event> delay_queue;

  /* add channels to fan */
  for ( unsigned int i = 0; i < delays.size(); i++ ) {
    fans.add_mature( prior );
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
  while ( (!delay_queue.empty()) || (fans.count_distinct() != 1) ) {
    steps++;

    if ( steps > 10000 ) {
      printf( "Error: Iterated %d steps to t=%f but fan has %d distinct realizations\n",
	      steps, fans.time(), fans.count_distinct() );

      printf( "===" );
      for ( unsigned int i = 0; i < fans.size(); i++ ) {
	printf( "Overall equality between 0 and %d: %d\n", i,
		fans.get_channel( 0 ).channel == fans.get_channel( i ).channel );

	if ( !(fans.get_channel( 0 ).channel == fans.get_channel( i ).channel) ) {
	  for ( unsigned int j = 0; i < fans.get_channel( 0 ).channel.size(); j++ ) {
	    printf( "0[%d] == %d[%d]? prob=%d, erased=%d, channel=%d\n",
		    j, i, j,
		    fans.get_channel( 0 ).channel.get_channel( j ).probability == fans.get_channel( i ).channel.get_channel( j ).probability,
		    fans.get_channel( 0 ).channel.get_channel( j ).erased == fans.get_channel( i ).channel.get_channel( j ).erased,
		    fans.get_channel( 0 ).channel.get_channel( j ).channel == fans.get_channel( i ).channel.get_channel( j ).channel );
	  }
	}
      }

      cout << fans.identify();
      exit( 1 );
    }

    double next_event_time = fans.next_time();

    if ( delay_queue.empty() ) {
      fans.advance_to( next_event_time );
    } else {
      Event next_send = delay_queue.top();
      fans.advance_to( MIN( next_send.time, next_event_time ) );

      if ( fans.time() == next_send.time ) {
	/* send */
	delay_queue.pop();

	assert( !sent_yet[ next_send.addr ] );
	for ( unsigned int i = 0; i < fans.get_channel( next_send.addr ).channel.size(); i++ ) {
	  if ( fans.get_channel( next_send.addr ).channel.get_channel( i ).delay != EARLY_WAKEUP ) {
	    extractor->get_pawn( fans.get_channel( next_send.addr ).channel.get_channel( i ).channel ).send( Packet( 12000, 0, 0, fans.time() ) );
	  }
	}
	sent_yet[ next_send.addr ] = true;
      }
    }

    /* iterate through channels */
    for ( unsigned int i = 0; i < fans.size(); i++ ) {
      for ( unsigned int j = 0; j < fans.get_channel( i ).channel.size(); j++ ) {
	if ( (!sent_yet[ i ]) && (extractor->get_collector( fans.get_channel( i ).channel.get_channel( j ).channel ).get_packets().size() != 0) ) {
	  fans.get_channel( i ).channel.get_channel( j ).delay = EARLY_WAKEUP; /* early wakeup same as non-sending */
	}

	double the_utility = utility( extractor->get_collector( fans.get_channel( i ).channel.get_channel( j ).channel ).get_packets() )
	+ utility( extractor->get_cross_traffic( fans.get_channel( i ).channel.get_channel( j ).channel ).get_packets() );
	fans.get_channel( i ).channel.get_channel( j ).utility += the_utility;

	extractor->reset( fans.get_channel( i ).channel.get_channel( j ).channel );
      }
    }

    /*
    printf( "===\nIterating at time %f, distinct = %d\n", fans.time(), fans.count_distinct() );
    cout << fans.identify();
    */
  }
  printf( "Sending solution found after %d steps\n", steps );

  /* total up utilities */
  priority_queue< pair< double, double > > strategies;

  for ( unsigned int i = 0; i < fans.size(); i++ ) {
    double total_probability = 0;
    double utility = 0;

    EmbeddableEnsemble<ChannelType> &ch = fans.get_channel( i ).channel;

    for ( unsigned int j = 0; j < ch.size(); j++ ) {
      utility += ch.get_channel( j ).utility * ch.get_channel( j ).probability;
      total_probability += ch.get_channel( j ).probability;
    }

    assert( fabs( total_probability - 1.0 ) <= 1e-10 );
    strategies.push( make_pair( utility, -fans.get_channel( i ).delay ) );
  }

  next_send_time = -strategies.top().second;

  if ( next_send_time >= base_time ) {
    container->sleep_until( next_send_time, addr, 99 );
  } else {
    container->sleep_until( base_time + LIMIT, addr, 99 );
  }

  /*
  while ( !strategies.empty() ) {
    printf( "At time %f, utility (%f/%f) comes from delay of %f\n",
	    container->time(), strategies.top().first,
	    fans.time() - base_time,
	    -strategies.top().second - container->time() );
    strategies.pop();
  }
  */
}

