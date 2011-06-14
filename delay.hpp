#ifndef DELAY_HPP
#define DELAY_HPP

#include <list>

#include "channel.hpp"

struct ScheduledPacket {
  double delivery_time;
  Packet packet;

  ScheduledPacket( double s_time, Packet s_pack )
    : delivery_time( s_time ), packet( s_pack )
  {}
};

class Delay : public Channel {
private:
  double delay;
  std::list<ScheduledPacket> contents;

public:
  Delay( double s_delay );

  void send( Packet pack );
  bool sendable( void ) { return true; }
  void wakeup( void );
};

#endif
