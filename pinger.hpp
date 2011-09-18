#ifndef PINGER_HPP
#define PINGER_HPP

#include "channel.hpp"
#include "container.hpp"

class Pinger : public Channel
{
private:
  double next_ping_time;
  double increment;
  int counter;
  
  int id;
  bool dummy; /* whether to make pings unique */

public:  
  Pinger( double s_increment, int s_id = 0, bool s_dummy = false );

  void init( void );
  void wakeup( void );
  void send( Packet pack ) { container->receive( addr, pack ); }
  void uncork( void ) { container->signal_sendable( addr ); }
  bool sendable( void ) { return container->can_send( addr ); }

  bool operator==( const Pinger &x ) const { return (next_ping_time == x.next_ping_time) && (increment == x.increment) && (counter == x.counter) && (id == x.id) && (dummy == x.dummy); }

  string identify( void ) const;

  friend size_t hash_value( Pinger const & ch );
};

#endif
