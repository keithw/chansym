#include <assert.h>
#include <stdio.h>

#include "ensemble_container.hpp"

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer()
  : the_time( 0 ), channels(), fork_queue(), erased_count( 0 )
{}

template <class ChannelType>
void EnsembleContainer<ChannelType>::add( ChannelType s_channel )
{
  assert( time() == 0 );

  for ( typename vector<WeightedChannel>::iterator i = channels.begin();
	i != channels.end();
	i++ ) {
    i->probability *= channels.size() / (double)( channels.size() + 1 );
  }
  
  channels.push_back( WeightedChannel( 1.0 / (double)( channels.size() + 1 ), s_channel ) );
  int new_addr = channels.size() - 1;
  channels[ new_addr ].channel.connect( new_addr, this );
  channels[ new_addr ].channel.init();
}


template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer( ChannelType s_channel )
  : the_time( 0 ), channels(), fork_queue(), erased_count( 0 )
{
  channels.push_back( WeightedChannel( 1.0, s_channel ) );
  channels[ 0 ].channel.connect( 0, this );
  channels[ 0 ].channel.init();
}

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer( const EnsembleContainer<ChannelType> &x )
  : Container( x ), the_time( x.the_time ), channels( x.channels ), erased_count( x.erased_count )
{
  for ( int i = 0; i < channels.size(); i++ ) {
    channels[ i ].channel.connect( i, this );
  }
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::execute_fork( void )
{
  if ( !fork_queue.empty() ) {
    PendingFork pending = fork_queue.front();   

    double p = channels[ pending.orig_addr ].probability;
    channels[ pending.orig_addr ].probability = p * pending.my_probability;

    WeightedChannel new_member( p * (1 - pending.my_probability), channels[ pending.orig_addr ].channel );

    channels.push_back( new_member );
    int new_addr = channels.size() - 1;
    channels[ new_addr ].channel.newaddr( new_addr, this );

    vector<Event> new_wakeups;

    /* duplicate old channel's wakeups */
    for ( peekable_priority_queue<Event, deque<Event>, Event>::const_iterator i = wakeups.begin();
	  i != wakeups.end();
	  i++ ) {
      if ( i->addr == pending.orig_addr ) {
	new_wakeups.push_back( Event( i->time, new_addr ) );
      }
    }

    for ( vector<Event>::const_iterator i = new_wakeups.begin();
	  i != new_wakeups.end();
	  i++ ) {
      wakeups.push( *i );
    }

    channels[ pending.orig_addr ].channel.after_fork( false, pending.fs );
    channels[ new_addr ].channel.after_fork( true, pending.fs );

    fork_queue.pop();
    assert( fork_queue.empty() );
  }  
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::combine( void )
{
  for ( unsigned int a1 = 0; a1 < channels.size(); a1++ ) {
    for ( unsigned int a2 = a1 + 1; a2 < channels.size(); a2++ ) {
      if ( channels[ a1 ].channel == channels[ a2 ].channel ) {
	/* compare wakeups */
	vector<double> wakeups1, wakeups2;
	for ( peekable_priority_queue<Event, deque<Event>, Event>::const_iterator i = wakeups.begin();
	      i != wakeups.end();
	      i++ ) {
	  if ( i->addr == (int)a1 ) {
	    wakeups1.push_back( i->time );
	  } else if ( i->addr == (int)a2 ) {
	    wakeups2.push_back( i->time );
	  }
	}

	if ( wakeups1 == wakeups2 ) {
	  /* compact */
	  channels[ a1 ].probability += channels[ a2 ].probability;
	  channels[ a2 ].probability = 0;
	  channels[ a2 ].erased = true;
	  erased_count++;
	}
      }
    }
  }
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::compact( void )
{
  vector<WeightedChannel> new_channels;
  peekable_priority_queue<Event, deque<Event>, Event> new_wakeups;

  for ( unsigned int addr = 0; addr < channels.size(); addr++ ) {
    if ( !channels[ addr ].erased ) {
      new_channels.push_back( channels[ addr ] );
      int new_addr = new_channels.size() - 1;
      new_channels.back().channel.newaddr( new_addr, this );
      for ( peekable_priority_queue<Event, deque<Event>, Event>::const_iterator i = wakeups.begin();
	    i != wakeups.end();
	    i++ ) {
	if ( i->addr == (int)addr ) {
	  new_wakeups.push( Event( i->time, new_addr ) );
	}
      }
    }
  }

  channels = new_channels;
  wakeups = new_wakeups;
  erased_count = 0;
}

template <class ChannelType>
bool EnsembleContainer<ChannelType>::tick( void )
{
  execute_fork();
  combine();

  if ( erased_count ) {
    compact();
  }

  if ( wakeups.empty() ) {
    return false;
  }

  Event next_event = wakeups.top();
  wakeups.pop();

  assert( next_event.time >= the_time );
  the_time = next_event.time;

  assert( next_event.addr >= 0 );
  assert( next_event.addr < (int)channels.size() );
  
  if ( !channels[ next_event.addr ].erased ) {
    channels[ next_event.addr ].channel.wakeup();
  }

  return true;
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::fork( int source_addr, double my_probability, Channel::ForkState *fs )
{
  assert( source_addr >= 0 );
  assert( source_addr < (int)channels.size() );

  typename ChannelType::ForkState *new_fs = dynamic_cast<typename ChannelType::ForkState *>( fs );
  assert( new_fs );

  channels[ source_addr ].channel.set_forking();

  assert( fork_queue.empty() );
  
  fork_queue.push( PendingFork( source_addr, my_probability, *new_fs ) );
  delete fs;
}

template <class ChannelType>
double EnsembleContainer<ChannelType>::probability( int source_addr )
{
  return channels[ source_addr ].probability;
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::receive( int source_addr, Packet pack )
{
  printf( "[Prob %.7f] At %.5f received packet id %d (sent %.5f) [from channel %d/%d (+%d erased)]\n",
	  probability( source_addr ),
	  time(), pack.id, pack.send_time,
	  source_addr, (int)channels.size() - erased_count, erased_count );
}
