#include <boost/functional/hash.hpp>
#include <iostream>

#include "isender.hpp"
#include "extractor.hpp"
#include "pawn.hpp"

#define        MIN(a,b) (((a)<(b))?(a):(b))

template <class ChannelType>
ISender<ChannelType>::ISender( EnsembleContainer<ChannelType> s_prior,
			       Extractor<ChannelType> *s_extractor )
  : prior( s_prior ),
    extractor( s_extractor ),
    latest_time( -1 ),
    next_ping_time( -1 ), increment( 1 ), counter( 0 ), id( 0 ), smallestsize( prior.size() )
{}

template <class ChannelType>
ISender<ChannelType>::ISender( const ISender<ChannelType> &x )
  : Channel( x ),
    prior( x.prior ), extractor( x.extractor ),
    latest_time( x.latest_time ),
    next_ping_time( x.next_ping_time ), increment( x.increment ), counter( x.counter ), id( x.id ), smallestsize( x.smallestsize )
{}

template <class ChannelType>
ISender<ChannelType> & ISender<ChannelType>::operator=( const ISender<ChannelType> &x )
{
  Channel::operator=( x );

  prior = x.prior;
  extractor = x.extractor;
  latest_time = x.latest_time;

  next_ping_time = x.next_ping_time;
  increment = x.increment;
  counter = x.counter;
  id = x.id;
  smallestsize = x.smallestsize;

  return *this;
}

template <class ChannelType>
void ISender<ChannelType>::init( void )
{
  next_ping_time = container->time();
  container->sleep_until( next_ping_time, addr, 99 );
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
       && (true_received.empty()) ) {
    return;
  } else {
    latest_time = current_time;
  }

  /* advance prior to actual time */
  prior.advance_to( current_time );

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
  /* Really send packet */
  container->receive( addr, p );

  /* Send packet in simulation */
  for ( unsigned int i = 0; i < prior.size(); i++ ) {
    if ( !prior.get_channel( i ).erased ) {
      extractor->get_pawn( prior.get_channel( i ).channel ).send( p ); /* Make him do this properly */
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

    util = 10 - delay;
  }

  return util;
}

template <class ChannelType>
void ISender<ChannelType>::optimal_action( void )
{
  assert( prior.size() == 1 );
  assert( container->time() == prior.time() );

  EnsembleContainer<ChannelType> silent = prior, sent = prior;

  extractor->get_pawn( sent.get_channel( 0 ).channel ).send( Packet( 12000, 0, 0, container->time() ) );

  printf( "Initial simulated time is %f/%f, equality is %d\n",
	  silent.time(), sent.time(), silent == sent );
    cout << silent.identify();
    cout << sent.identify();
    printf( "\n\n\n" );

    double silentu = 0, sentu = 0;

  while ( 1 ) {
    double next_time = MIN( silent.next_time(), silent.next_time() );
    silent.advance_to( next_time );
    sent.advance_to( next_time );

    silentu += utility( extractor->get_collector( silent.get_channel( 0 ).channel ).get_packets() )
      + 5 * utility( extractor->get_cross_traffic( silent.get_channel( 0 ).channel ).get_packets() );

    sentu += utility( extractor->get_collector( sent.get_channel( 0 ).channel ).get_packets() )
      + 5 * utility( extractor->get_cross_traffic( sent.get_channel( 0 ).channel ).get_packets() );

    extractor->reset( silent.get_channel( 0 ).channel );
    extractor->reset( sent.get_channel( 0 ).channel );

    printf( "At simulated time %f/%f, utility is %f/%f, silent and sent equality is: %d\n",
	    silent.time(), sent.time(),
	    silentu, sentu, silent == sent );
    printf( "SENDING ADVANTAGE = %f\n", sentu - silentu );
    cout << silent.identify();
    cout << sent.identify();
    printf( "\n\n\n" );
  }
}