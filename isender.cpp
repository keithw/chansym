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
    collector( NULL ),
    next_ping_time( -1 ), increment( 1 ), counter( 0 ), id( 0 ), smallestsize( prior.size() )
{}

template <class ChannelType>
ISender<ChannelType>::ISender( const ISender<ChannelType> &x )
  : Channel( x ),
    prior( x.prior ), extractor( x.extractor ), collector( x.collector ),
    next_ping_time( x.next_ping_time ), increment( x.increment ), counter( x.counter ), id( x.id ), smallestsize( x.smallestsize )
{}

template <class ChannelType>
ISender<ChannelType> & ISender<ChannelType>::operator=( const ISender<ChannelType> &x )
{
  Channel::operator=( x );

  prior = x.prior;
  extractor = x.extractor;
  collector = x.collector;
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

  /* advance prior to actual time */
  prior.advance_to( current_time );

  /* find and kill mismatches */
  for ( unsigned int i = 0; i < prior.size(); i++ ) {
    if ( prior.get_channel( i ).erased ) {
      continue;
    }

    if ( extractor->get_collector( prior.get_channel( i ).channel ).get_packets()
	 != collector->get_packets() ) {
      printf( "At time %f, true size = %d and prior[%d] size = %d\n",
	      current_time, (int)collector->get_packets().size(),
	      i, (int)extractor->get_collector( prior.get_channel( i ).channel ).get_packets().size() );	       

      prior.erase( i );
    }

    extractor->get_collector( prior.get_channel( i ).channel ).reset();
  }

  /* reset real collector */
  collector->reset();
  assert( collector->get_packets().size() == 0 );

  prior.prune( 1000 );

  prior.normalize();

  if ( (prior.size() >= 2 * smallestsize) || (prior.get_erased_count() * 2 >= (int)prior.size()) ) {
    prior.combine();
    smallestsize = prior.size();
  }

  printf( "Time: %f (channels: %d)\n", current_time, prior.size() );
  if ( prior.size() <= 240000 ) {
    cout << prior.identify();
  }

  if ( current_time == next_ping_time ) {
    sendout( Packet( 12000, id, counter++, current_time ) );
    
    next_ping_time += increment;
    container->sleep_until( next_ping_time, addr );
  }

  container->sleep_until( prior.next_time(), addr );
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
      extractor->get_pawn( prior.get_channel( i ).channel ).send( p );
    }
  }

  prior.execute_fork();
}
