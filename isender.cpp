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
    next_send_time( 0 ),
    id( 0 ),
    vi()
{}

template <class ChannelType>
ISender<ChannelType>::ISender( const ISender<ChannelType> &x )
  : Channel( x ),
    prior( x.prior ), extractor( x.extractor ),
    latest_time( x.latest_time ),
    next_send_time( x.next_send_time ),
    id( x.id ),
    vi( x.vi )
{}

template <class ChannelType>
ISender<ChannelType> & ISender<ChannelType>::operator=( const ISender<ChannelType> &x )
{
  Channel::operator=( x );

  prior = x.prior;
  extractor = x.extractor;
  latest_time = x.latest_time;

  next_send_time = x.next_send_time;

  id = x.id;

  vi = x.vi;

  return *this;
}

template <class ChannelType>
void ISender<ChannelType>::init( void )
{
  assert( container->time() == 0 );
  container->sleep_until( 0, addr, 99 );
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
    sendout( Packet( 12000, id, 0, current_time ) );
    next_send_time = current_time + 1;
    container->sleep_until( next_send_time, addr, 99 );
  }

  value_experiment();

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

  //  prior.prune( 1000 );

  prior.normalize();

  prior.heuristic_opportunistic_combine();

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

template <class ChannelType>
void ISender<ChannelType>::value_experiment( void )
{
  for ( unsigned int i = 0; i < prior.size(); i++ ) {
    printf( "Valuing channel %d of %d...\n", i, prior.size() );
    double val = vi.value( prior.get_channel( i ).channel );
    printf( "value = %f\n", val );
  }
}
