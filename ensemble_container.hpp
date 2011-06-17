#ifndef ENSEMBLE_CONTAINER_HPP
#define ENSEMBLE_CONTAINER_HPP

#include <queue>
#include "container.hpp"

using namespace std;

template <class ChannelType>
class WeightedChannel {
public:
  double probability;
  ChannelType channel;

  WeightedChannel( double s_p, ChannelType s_c )
    : probability( s_p ), channel( s_c )
  {}

  bool operator==( const WeightedChannel &x ) const
  {
    return (probability == x.probability) && (channel == x.channel);
  }
};

template <class ChannelType>
class PendingFork {
public:
  int orig_addr;
  double my_probability;
  ChannelType other;
  typename ChannelType::ForkState fs;

  PendingFork( int s_orig_addr, double s_my_prob, ChannelType s_other, typename ChannelType::ForkState s_fs )
    : orig_addr( s_orig_addr ), my_probability( s_my_prob ),
      other( s_other ), fs( s_fs )
  {}

  bool operator==( const PendingFork &x ) const
  {
    return (orig_addr == x.orig_addr) && (my_probability == x.my_probability)
      && (other == x.other) && (fs == x.fs);
  }
};

template <class ChannelType>
class EnsembleContainer : public Container
{
private:
  double the_time;
  vector<WeightedChannel<ChannelType>> channels;
  queue<PendingFork<ChannelType>> fork_queue;

public:
  EnsembleContainer( ChannelType s_channel );
  EnsembleContainer( const EnsembleContainer<ChannelType> &x );

  bool tick( void );

  void sleep_until( double time, int source_addr ) { wakeups.push( Event( time, source_addr ) ); }
  void signal_sendable( int ) {}
  bool can_send( int ) { return( true ); }
  void receive( int, Packet ) {}
  double time( void ) { return the_time; }
  void fork( int source_addr, double my_probability, Channel *other, Channel::ForkState *fs );
  double probability( int source_addr );

  bool operator==( const EnsembleContainer<ChannelType> &x ) const { return (the_time == x.the_time) && (channels == x.channels); }
};

#endif