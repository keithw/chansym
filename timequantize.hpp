#ifndef TIMEQUANTIZE_HPP
#define TIMEQUANTIZE_HPP

#include <list>

#include "channel.hpp"

class TimeQuantize : public Channel {
private:
  double interval;
  std::list<ScheduledPacket> contents;

public:
  TimeQuantize( double s_delay );

  void send( Packet pack );
  bool sendable( void ) { return true; }
  void wakeup( void );

  bool operator==( const TimeQuantize &x ) const { return (interval == x.interval) && (contents == x.contents); }

  string identify( void ) const;

  friend size_t hash_value( TimeQuantize const & ch );
};

#endif
