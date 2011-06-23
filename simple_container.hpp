#ifndef SIMPLE_CONTAINER_HPP
#define SIMPLE_CONTAINER_HPP

#include "container.hpp"

#include <queue>
#include <assert.h>

template <class ChannelType>
class SimpleContainer : public Container
{
private:
  class PendingFork {
  public:
    double my_probability;
    typename ChannelType::ForkState fs;

    PendingFork( double s_my_prob, typename ChannelType::ForkState s_fs )
      : my_probability( s_my_prob ), fs( s_fs )
    {}

    bool operator==( const PendingFork &x ) const
    {
      return (my_probability == x.my_probability) && (fs == x.fs);
    }
  };

  double the_time;
  ChannelType channel;
  double the_probability;
  queue<PendingFork> fork_queue;

  void execute_fork( void );

public:
  SimpleContainer( ChannelType s_channel );
  SimpleContainer( const SimpleContainer<ChannelType> &x );

  bool tick( void );
  ChannelType & get_chan( void ) { return channel; }

  void sleep_until( double time, int source_addr ) { wakeups.push( Event( time, source_addr ) ); }
  void signal_sendable( int ) {}
  bool can_send( int ) { return( true ); }
  void receive( int source_addr, Packet pack );
  double time( void ) { return the_time; }
  void fork( int source_addr, double my_probability, Channel::ForkState *fs );
  double probability( int ) { return the_probability; }

  bool operator==( const SimpleContainer<ChannelType> &x ) const { return (the_time == x.the_time) && (channel == x.channel); }

  string identify( void );
};

#endif
