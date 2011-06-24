#ifndef ENSEMBLE_CONTAINER_HPP
#define ENSEMBLE_CONTAINER_HPP

#include <queue>
#include "container.hpp"

using namespace std;

template <class ChannelType>
class EnsembleContainer : public Container
{
private:
  class WeightedChannel {
  public:
    double probability;
    ChannelType channel;
    bool erased;

    WeightedChannel( double s_p, ChannelType s_c )
      : probability( s_p ), channel( s_c ), erased( false )
    {}

    bool operator==( const WeightedChannel &x ) const
    {
      return (probability == x.probability) && (channel == x.channel) && (erased == x.erased);
    }
  };

  class PendingFork {
  public:
    int orig_addr;
    double my_probability;
    typename ChannelType::ForkState fs;

    PendingFork( int s_orig_addr, double s_my_prob, typename ChannelType::ForkState s_fs )
      : orig_addr( s_orig_addr ), my_probability( s_my_prob ), fs( s_fs )
    {}

    bool operator==( const PendingFork &x ) const
    {
      return (orig_addr == x.orig_addr) && (my_probability == x.my_probability) && (fs == x.fs);
    }
  };

  double the_time;
  vector<WeightedChannel> channels;
  queue<PendingFork> fork_queue;
  int erased_count;

  bool printing;
  bool forking;

  void execute_fork( void );
  void compact( void );
  void assert_normalized( void );

public:
  EnsembleContainer();
  EnsembleContainer( ChannelType s_channel );
  EnsembleContainer( const EnsembleContainer<ChannelType> &x );

  bool tick( void );

  void combine( void );

  void sleep_until( double time, int source_addr ) { wakeups.push( Event( time, source_addr ) ); }
  void signal_sendable( int ) {}
  bool can_send( int ) { return( true ); }
  void receive( int source_addr, Packet pack );
  double time( void ) { return the_time; }
  void fork( int source_addr, double my_probability, Channel::ForkState *fs );
  double probability( int source_addr );

  void add( ChannelType s_channel );
  void set_printing( bool s_printing ) { printing = s_printing; }
  void set_forking( bool s_forking ) { forking = s_forking; }

  bool operator==( const EnsembleContainer<ChannelType> &x ) const { return (the_time == x.the_time) && (channels == x.channels); }

  unsigned int size( void ) { return channels.size(); }
  WeightedChannel & get_channel( int address );

  bool live( void ) { return !wakeups.empty(); }
  double next_time( void );
  void erase( int address );
  void normalize( void );
  void prune( double threshold );

  string identify( void );
};

#endif
