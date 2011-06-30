#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <google/dense_hash_map>
#include <boost/functional/hash.hpp>
#include <boost/optional/optional.hpp>

#include "ensemble_container.hpp"

using google::dense_hash_map;

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer()
  : the_time( 0 ), channels(), fork_queue(), erased_count( 0 ), printing( false ), forking( true )
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
  : the_time( 0 ), channels(), fork_queue(), erased_count( 0 ), printing( false ), forking( true )
{
  channels.push_back( WeightedChannel( 1.0, s_channel ) );
  channels[ 0 ].channel.connect( 0, this );
  channels[ 0 ].channel.init();
}

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer( const EnsembleContainer<ChannelType> &x )
  : Container( x ), the_time( x.the_time ), channels( x.channels ), erased_count( x.erased_count ), printing( x.printing ), forking( x.forking )
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

    if ( forking ) {
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
    } else {
      assert( channels.size() == 1 );
      assert( pending.orig_addr == 0 );
      
      int threshold = pending.my_probability * RAND_MAX;
      if ( rand() < threshold ) {
	channels[ 0 ].probability *= pending.my_probability;
	channels[ 0 ].channel.after_fork( false, pending.fs );
      } else {
	channels[ 0 ].probability *= (1 - pending.my_probability);
	channels[ 0 ].channel.after_fork( true, pending.fs );
      }
    }

    fork_queue.pop();
    assert( fork_queue.empty() );
  }  
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::erase( int address )
{
  assert( address >= 0 );
  assert( address < (int)channels.size() );

  channels[ address ].probability = 0;
  if ( !channels[ address ].erased ) erased_count++;
  channels[ address ].erased = true;
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::normalize( void )
{
  /* get total */
  double total = 0;

  for ( typename vector<WeightedChannel>::const_iterator i = channels.begin();
	i != channels.end();
	i++ ) {
    total += i->probability;
  }

  assert( total > 0 );

  /* normalize */
  for ( typename vector<WeightedChannel>::iterator i = channels.begin();
	i != channels.end();
	i++ ) {
    i->probability *= 1.0 / total;
  }
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::assert_normalized( void )
{
  double new_total = 0;

  for ( typename vector<WeightedChannel>::const_iterator i = channels.begin();
	i != channels.end();
	i++ ) {
    new_total += i->probability;
  }

  assert( fabs( new_total - 1.0 ) < 1e-10 );
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::combine( void )
{
  typedef ChannelType * key_t;
  typedef dense_hash_map< key_t, size_t, channel_hash, equal_channels > dhm_t;
  assert( (int)channels.size() > erased_count );
  dhm_t set( (int)channels.size() - erased_count );
  set.set_empty_key( key_t() );

  for ( unsigned int a1 = 0; a1 < channels.size(); a1++ ) {
    if ( channels[ a1 ].erased ) {
      continue;
    }

    key_t key( &(channels[ a1 ].channel) );
    typename dhm_t::const_iterator it = set.find( key );

    if ( it == set.end() ) {
      set[ key ] = a1;
    } else {
      size_t first_channel = set[ key ];

      assert( first_channel != a1 );
      channels[ first_channel ].probability += channels[ a1 ].probability;
      erase( a1 );
    }
  }

  if ( erased_count ) {
    compact();
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
    } else {
      assert( channels[ addr ].probability == 0 );
    }
  }

  channels = new_channels;
  wakeups = new_wakeups;
  erased_count = 0;
}

template <class ChannelType>
double EnsembleContainer<ChannelType>::next_time( void )
{
  assert( live() );
  return wakeups.top().time;
}

template <class ChannelType>
bool EnsembleContainer<ChannelType>::tick( void )
{
  if ( wakeups.empty() ) {
    return false;
  }

  if ( forking ) {
    assert_normalized();
  } else {
    assert( channels.size() == 1 );
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

  execute_fork();

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
void EnsembleContainer<ChannelType>::receive( int source_addr __attribute((unused)), Packet pack __attribute((unused)) )
{
  if ( printing ) {
    printf( "[Prob %.7f] At %.5f received packet id %d (sent %.5f by src %d) [from channel %d/%d (+%d erased)]\n",
	    probability( source_addr ),
	    time(), pack.id, pack.send_time, pack.src,
	    source_addr, (int)channels.size() - erased_count, erased_count );
  }
}

template <class ChannelType>
typename EnsembleContainer<ChannelType>::WeightedChannel & EnsembleContainer<ChannelType>::get_channel( int address )
{
  assert( address >= 0 );
  assert( address < (int)channels.size() );

  return channels[ address ];
}

template <class ChannelType>
string EnsembleContainer<ChannelType>::identify( void )
{
  ostringstream response;

  response << "<Ensemble (size=";
  response << (int)channels.size() - erased_count;
  response << ")\n";

  for ( unsigned int i = 0; i < size(); i++ ) {
    if ( channels[ i ].probability < 1.0 / (100 * channels.size()) ) {
      continue;
    }

    response << i;
    response << " (p=";
    response << channels[ i ].probability;
    response << "): " + channels[ i ].channel.identify();
    response << endl;
  }

  response << "</Ensemble (size=";
  response << (int)channels.size() - erased_count;
  response << ">\n";

  return response.str();
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::prune( double threshold )
{
  for ( unsigned int a1 = 0; a1 < channels.size(); a1++ ) {
    if ( channels[ a1 ].probability < 1.0 / (threshold * channels.size()) ) {
      erase( a1 );
    }
  }
}
