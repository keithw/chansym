#ifndef SIMPLE_CONTAINER_HPP
#define SIMPLE_CONTAINER_HPP

#include "container.hpp"

#include <assert.h>

template <class ChannelType>
class SimpleContainer : public Container
{
private:
  double the_time;
  ChannelType channel;

public:
  SimpleContainer( ChannelType s_channel );
  SimpleContainer( const SimpleContainer<ChannelType> &x );

  bool tick( void );
  ChannelType *get_chan( void ) { return &channel; }

  void sleep_until( double time, int source_addr ) { wakeups.push( Event( time, source_addr ) ); }
  void signal_sendable( int ) {}
  bool can_send( int ) { return( true ); }
  void receive( int, Packet ) {}
  double time( void ) { return the_time; }
  void fork( int , double , Channel * ) { assert( false ); }

  bool operator==( const SimpleContainer<ChannelType> &x ) const { return (the_time == x.the_time) && (channel == x.channel); }
};

#endif
