#include <boost/functional/hash.hpp>
#include <iostream>

#include "isender.hpp"
#include "extractor.hpp"
#include "pawn.hpp"

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
  container->sleep_until( next_ping_time, addr );
}

template <class ChannelType>
void ISender<ChannelType>::wakeup( void )
{
  double current_time = container->time();

  assert( current_time >= latest_time );
  if ( current_time == latest_time ) {
    return;
  } else {
    latest_time = current_time;
  }

  /* advance prior to actual time */
  prior.advance_to( current_time );

  /* ping if necessary */
  if ( current_time == next_ping_time ) {
    sendout( Packet( 12000, id, counter++, current_time ) );
    
    next_ping_time += increment;
    container->sleep_until( next_ping_time, addr );
  }

  container->sleep_until( prior.next_time(), addr );

  /* find true collector contents */
  Collector *collector = &extractor->get_collector( this );
  vector<ScheduledPacket> true_received = collector->get_packets();

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
