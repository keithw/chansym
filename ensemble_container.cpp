#include <assert.h>
#include <stdio.h>

#include "ensemble_container.hpp"

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer( ChannelType s_channel )
  : the_time( 0 ), channels(), fork_queue()
{
  channels.reserve( 1024 );

  channels.push_back( WeightedChannel<ChannelType>( 1.0, s_channel ) );
  channels[ 0 ].channel.connect( 0, this );
  channels[ 0 ].channel.init();
}

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer( const EnsembleContainer<ChannelType> &x )
  : Container( x ), the_time( x.the_time ), channels( x.channels )
{
  for ( int i = 0; i < channels.size(); i++ ) {
    channels[ i ].channel.connect( i, this );
  }
}

template <class ChannelType>
bool EnsembleContainer<ChannelType>::tick( void )
{
  if ( !fork_queue.empty() ) {
    PendingFork<ChannelType> pending = fork_queue.front();   
    fork_queue.pop();

    double p = channels[ pending.orig_addr ].probability;
    channels[ pending.orig_addr ].probability = p * pending.my_probability;

    WeightedChannel<ChannelType> new_member( p * (1 - pending.my_probability), pending.other );
    channels.push_back( new_member );

    int new_addr = channels.size() - 1;

    /* duplicate old channel's wakeups */
    for ( peekable_priority_queue<Event, deque<Event>, Event>::const_iterator i = wakeups.begin();
	  i != wakeups.end();
	  i++ ) {
      fprintf( stderr, "Futurewakeup..." );
      if ( i->addr == pending.orig_addr ) {
	wakeups.push( Event( i->time, new_addr ) );
	fprintf( stderr, "copied\n" );
      } else {
	fprintf( stderr, "not copied.\n" );
      }
    }
  }

  if ( wakeups.empty() ) {
    return false;
  }

  Event next_event = wakeups.top();
  wakeups.pop();

  assert( next_event.time >= the_time );
  the_time = next_event.time;

  fprintf( stderr, "Wakeup for channel #%d\n", next_event.addr );

  assert( next_event.addr >= 0 );
  assert( next_event.addr < (int)channels.size() );
  channels[ next_event.addr ].channel.wakeup();

  return true;
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::fork( int source_addr, double my_probability, Channel *other, Channel::ForkState *fs )
{
  assert( source_addr >= 0 );
  assert( source_addr < (int)channels.size() );

  ChannelType *new_other = dynamic_cast<ChannelType *>( other );
  typename ChannelType::ForkState *new_fs = dynamic_cast<typename ChannelType::ForkState *>( fs );

  assert( new_other && new_fs );

  assert( fork_queue.empty() );

  fork_queue.push( PendingFork<ChannelType>( source_addr, my_probability, *new_other, *new_fs ) );
  delete other;
  delete fs;
}

template <class ChannelType>
double EnsembleContainer<ChannelType>::probability( int source_addr )
{
  return channels[ source_addr ].probability;
}
