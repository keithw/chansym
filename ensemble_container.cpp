#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <google/dense_hash_map>
#include <google/dense_hash_set>
#include <boost/functional/hash.hpp>
#include <boost/optional/optional.hpp>

#include "ensemble_container.hpp"
#include "utility.hpp"

using google::dense_hash_map;
using google::dense_hash_set;

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer()
  : the_time( 0 ), channels(), fork_queue(), erased_count( 0 ), printing( false ), forking( true ), total_utility( 0 ), smallest_size( 0 )
{}

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer( double s_time )
  : the_time( s_time ), channels(), fork_queue(), erased_count( 0 ), printing( false ), forking( true ), total_utility( 0 ), smallest_size( 0 )
{}

template <class ChannelType>
void EnsembleContainer<ChannelType>::add( ChannelType s_channel )
{
  assert( time() == 0 );

  channels.push_back( WeightedChannel( 1.0, s_channel ) );
  int new_addr = channels.size() - 1;
  channels[ new_addr ].channel.connect( new_addr, this );
  channels[ new_addr ].channel.init();
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::add_mature( ChannelType s_channel )
{
  channels.push_back( WeightedChannel( 1.0, s_channel ) );
  int new_addr = channels.size() - 1;
  channels[ new_addr ].channel.connect( new_addr, this );

  /* duplicate old channel's wakeups */
  for ( typename ChannelType::wakeup_iterator i = channels[ new_addr ].channel.wakeup_begin();
	i != channels[ new_addr ].channel.wakeup_end();
	i++ ) {
    assert( i->time >= time() );
    make_wakeup( i->time, new_addr, i->sort_order );
  }
}

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer( ChannelType s_channel )
  : the_time( 0 ), channels(), fork_queue(), erased_count( 0 ), printing( false ), forking( true ), total_utility( 0 ), smallest_size( 0 )
{
  channels.push_back( WeightedChannel( 1.0, s_channel ) );
  channels[ 0 ].channel.connect( 0, this );
  channels[ 0 ].channel.init();
}

template <class ChannelType>
EnsembleContainer<ChannelType>::EnsembleContainer( const EnsembleContainer<ChannelType> &x )
  : Container( x ), the_time( x.the_time ), channels( x.channels ), fork_queue( x.fork_queue ), erased_count( x.erased_count ), printing( x.printing ), forking( x.forking ), total_utility( x.total_utility ), smallest_size( x.smallest_size )
{
  for ( unsigned int i = 0; i < channels.size(); i++ ) {
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
      new_member.utility = channels[ pending.orig_addr ].utility;

      channels.push_back( new_member );
      int new_addr = channels.size() - 1;
      channels[ new_addr ].channel.newaddr( new_addr, this );

      /* duplicate old channel's wakeups */
      for ( typename ChannelType::wakeup_iterator i = channels[ pending.orig_addr ].channel.wakeup_begin();
	    i != channels[ pending.orig_addr ].channel.wakeup_end();
	    i++ ) {
	make_wakeup( i->time, new_addr, i->sort_order );
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

  assert( close( new_total, 1.0 ) );
}

template <class ChannelType>
bool EnsembleContainer<ChannelType>::converged( void )
{
  assert( !channels.empty() );
  if ( channels.size() == 1 ) {
    return true;
  }

  for ( unsigned int i = 1; i < channels.size(); i++ ) {
    if ( !(channels[ 0 ].channel == channels[ i ].channel) ) {
      return false;
    }
  }

  return true;

  /*
  uint distinct = 0;
  for ( unsigned int i = 0; i < channels.size(); i++ ) {
    bool is_distinct = 1;
    for ( unsigned int j = 0; j < i; j++ ) {
      if ( channels[ j ].channel == channels[ i ].channel ) {
      // has predicate
	is_distinct = 0;
	break;
      }
    }
    distinct += is_distinct;
  }

  return distinct;

*/

  /*
  typedef ChannelType * key_t;
  typedef dense_hash_set< key_t, channel_hash, equal_channels > dhs_t;
  assert( (int)channels.size() > erased_count );
  dhs_t set( (int)channels.size() - erased_count );
  set.set_empty_key( NULL );

  for ( unsigned int a1 = 0; a1 < channels.size(); a1++ ) {
    if ( channels[ a1 ].erased ) {
      continue;
    }

    key_t key( &(channels[ a1 ].channel) );
    set.insert( key );
  }

  return set.size();
  */
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::heuristic_opportunistic_combine( void )
{
  if ( (size() >= 2 * smallest_size)
       || (erased_count * 2 >= (int)size()) ) {
    combine();
    smallest_size = size();
  }
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::combine( void )
{
  if ( (size() == 1) && (erased_count == 0) ) {
    return;
  }

  typedef ChannelType * key_t;
  typedef dense_hash_map< key_t, size_t, channel_hash, equal_channels > dhm_t;
  assert( (int)channels.size() > erased_count );
  dhm_t set( (int)channels.size() - erased_count );
  set.set_empty_key( NULL );

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
      channels[ first_channel ].utility = (channels[ first_channel ].probability * channels[ first_channel ].utility + channels[ a1 ].probability * channels[ a1 ].utility)
	/ (channels[ first_channel ].probability + channels[ a1 ].probability );
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

  vector<size_t> mapping( channels.size() );

  for ( unsigned int addr = 0; addr < channels.size(); addr++ ) {
    if ( !channels[ addr ].erased ) {
      new_channels.push_back( channels[ addr ] );
      int new_addr = new_channels.size() - 1;
      new_channels.back().channel.newaddr( new_addr, this );
      mapping[ addr ] = new_addr;
    } else {
      assert( channels[ addr ].probability == 0 );
    }
  }

  peekable_priority_queue<Event, deque<Event>, Event> old_wakeups = wakeups;
  clear_wakeups();

  for ( peekable_priority_queue<Event, deque<Event>, Event>::const_iterator i = old_wakeups.begin();
	i != old_wakeups.end();
	i++ ) {
    if ( !channels[ i->addr ].erased ) {
      make_wakeup( i->time, mapping[ i->addr ], i->sort_order );
    }
  }

  channels = new_channels;
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
#ifdef STRONG_ASSERTS
    assert_normalized();
#endif
  } else {
    assert( channels.size() == 1 );
  }

  Event next_event = wakeups.top();
  wakeups.pop();

  assert( next_event.time >= time() );
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
    vector<ScheduledPacket> real, cross;
    if ( pack.src == 0 ) {
      real.push_back( ScheduledPacket( time(), pack ) );
    } else {
      cross.push_back( ScheduledPacket( time(), pack ) );
    }
    total_utility += UtilityMetric::utility( time(), real, cross );

    printf( "[Prob %.7f] At %f received packet id %d (sent %f by src %d, delayed %f) [from channel %d/%d (+%d erased)] Utility now: %f\n",
	    probability( source_addr ),
	    time(), pack.id, pack.send_time, pack.src, (time() - pack.send_time),
	    source_addr, (int)channels.size() - erased_count, erased_count, total_utility );
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
string EnsembleContainer<ChannelType>::identify( void ) const
{
  ostringstream response;

  response << "<Ensemble (size=";
  response << (int)channels.size() - erased_count;
  response << ")\n";

  for ( unsigned int i = 0; i < size(); i++ ) {
    if ( channels[ i ].probability < .01 ) {
      continue;
    }

    if ( channels[ i ].erased ) {
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

template <class ChannelType>
size_t EnsembleContainer<ChannelType>::hash( void ) const
{
  assert( fork_queue.empty() );

  size_t seed = 0;
  boost::hash_combine( seed, wakeups );
  boost::hash_combine( seed, time() );
  boost::hash_combine( seed, channels );
  boost::hash_combine( seed, forking );
  return seed;
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::advance_to( double advance_time )
{
  assert( advance_time >= time() );

  while ( live() && (next_time() <= advance_time) ) {
    tick();
  }

  the_time = advance_time;
}

template <class ChannelType>
bool EnsembleContainer<ChannelType>::operator==( const EnsembleContainer<ChannelType> &x ) const
{
  if ( !close( time(), x.time() ) )
    return false;
  
  if ( !(fork_queue.empty() && x.fork_queue.empty()) )
    return false;
  
  if ( channels.size() != x.channels.size() )
    return false;
  
  /* Now compare channel sets */
  typedef const WeightedChannel * key_t;
  typedef dense_hash_set< key_t, channel_hash, equal_channels > dhs_t;
  
  dhs_t my_set( (int)channels.size() - erased_count );
  dhs_t other_set( (int)x.channels.size() - x.erased_count );

  my_set.set_empty_key( NULL );
  other_set.set_empty_key( NULL );

  for ( unsigned i = 0; i < channels.size(); i++ ) {
    if ( !channels[ i ].erased ) {
      key_t key( &channels[ i ] );
      my_set.insert( key );
    }

    if ( !x.channels[ i ].erased ) {
      key_t key( &x.channels[ i ] );
      other_set.insert( key );
    }
  }
  
  for ( unsigned i = 0; i < channels.size(); i++ ) {
    if ( !channels[ i ].erased ) {
      key_t key( &channels[ i ] );
      typename dhs_t::const_iterator it = other_set.find( key );
      if ( it == other_set.end() ) {
	return false;
      }
    }

    if ( !x.channels[ i ].erased ) {
      key_t key( &x.channels[ i ] );
      typename dhs_t::const_iterator it = my_set.find( key );
      if ( it == my_set.end() ) {
	return false;
      }
    }
  }
  
  return true;
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::clear_wakeups( int source_addr )
{
  peekable_priority_queue<Event, deque<Event>, Event> old_wakeups = wakeups;
  clear_wakeups();
  
  for ( peekable_priority_queue<Event, deque<Event>, Event>::const_iterator i = old_wakeups.begin();
	i != old_wakeups.end();
	i++ ) {
    if ( i->addr != source_addr ) {
      make_wakeup( i->time, i->addr, i->sort_order );
    }
  }
}

template <class ChannelType>
void EnsembleContainer<ChannelType>::print_wakeups( void )
{
  printf( "WAKEUPS (%d channels): ", (unsigned int)channels.size() );
  peekable_priority_queue<Event, deque<Event>, Event> new_wakeups = wakeups;

  while ( !new_wakeups.empty() ) {
    printf( "<addr=%d, time=%f> ", new_wakeups.top().addr, new_wakeups.top().time );
    new_wakeups.pop();
  }

  printf( "\n" );
}
