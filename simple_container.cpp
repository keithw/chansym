#include <assert.h>
#include <stdlib.h>

#include "simple_container.hpp"

template <class ChannelType>
SimpleContainer<ChannelType>::SimpleContainer( ChannelType s_channel )
  : the_time( 0 ), channel( s_channel ), the_probability( 1.0 ), fork_queue()
{
  channel.connect( 0, this );
  channel.init();
}

template <class ChannelType>
SimpleContainer<ChannelType>::SimpleContainer( const SimpleContainer<ChannelType> &x )
  : Container( x ), the_time( x.the_time ), channel( x.channel ), the_probability( x.the_probability ), fork_queue( x.fork_queue )
{
  channel.connect( 0, this );
}

template <class ChannelType>
bool SimpleContainer<ChannelType>::tick( void )
{
  execute_fork();

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

template <class ChannelType>
void SimpleContainer<ChannelType>::execute_fork( void )
{
  if ( !fork_queue.empty() ) {
    PendingFork pending = fork_queue.front();

    int threshold = pending.my_probability * RAND_MAX;
    if ( rand() < threshold ) {
      the_probability *= pending.my_probability;
      channel.after_fork( false, pending.fs );
    } else {
      the_probability *= (1 - pending.my_probability);
      channel.after_fork( true, pending.fs );
    }

    fork_queue.pop();
    assert( fork_queue.empty() );
  }
}

template <class ChannelType>
void SimpleContainer<ChannelType>::fork( int source_addr, double my_probability, Channel::ForkState *fs )
{
  assert( source_addr == 0 );

  typename ChannelType::ForkState *new_fs = dynamic_cast<typename ChannelType::ForkState *>( fs );
  assert( new_fs );

  channel.set_forking();

  assert( fork_queue.empty() );

  fork_queue.push( PendingFork( my_probability, *new_fs ) );
  delete fs;  
}

template <class ChannelType>
void SimpleContainer<ChannelType>::receive( int source_addr __attribute((unused)), Packet pack __attribute((unused)) )
{
  /*
  printf( "[Prob %.7f] At %.5f received packet id %d (sent %.5f)\n",
	  probability( source_addr ),
	  time(), pack.id, pack.send_time );
  */
}
