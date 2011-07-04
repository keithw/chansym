#ifndef DELAY_HPP
#define DELAY_HPP

#include <list>

#include "channel.hpp"

class Delay : public Channel {
private:
  double delay;
  std::list<ScheduledPacket> contents;

public:
  Delay( double s_delay );

  void send( Packet pack );
  bool sendable( void ) { return true; }
  void wakeup( void );

  bool operator==( const Delay &x ) const { return (delay == x.delay) && (contents == x.contents); }

  string identify( void );

  friend size_t hash_value( Delay const & ch );
};

#endif
