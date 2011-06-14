#ifndef PINGER_HPP
#define PINGER_HPP

#include "channel.hpp"

class Pinger : public Channel
{
private:
  double next_ping_time;
  double increment;
  int counter;
  
public:  
  Pinger( double s_increment );

  void init( void );
  void wakeup( void );
  void send( Packet ) { assert( false ); }
  bool sendable( void ) { return false; }

  bool operator==( const Pinger &x ) const { return (next_ping_time == x.next_ping_time) && (increment == x.increment) && (counter == x.counter); }
};

#endif
