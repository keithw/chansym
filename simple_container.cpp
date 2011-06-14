#include <assert.h>

#include "simple_container.hpp"

template <class ChannelType>
SimpleContainer<ChannelType>::SimpleContainer( ChannelType s_channel )
  : the_time( 0 ), channel( s_channel )
{
  channel.connect( 0, this );
  channel.init();
}

template <class ChannelType>
SimpleContainer<ChannelType>::SimpleContainer( const SimpleContainer<ChannelType> &x )
  : Container( x ), the_time( x.the_time ), channel( x.channel )
{
  channel.connect( 0, this );
}

template <class ChannelType>
bool SimpleContainer<ChannelType>::tick( void )
{
  if ( wakeups.empty() ) {
    return false;
  }

  Event next_event = wakeups.top();
  wakeups.pop();

  assert( next_event.time >= the_time );
  the_time = next_event.time;

  assert( next_event.addr == 0 );
  channel.wakeup();

  return true;
}
