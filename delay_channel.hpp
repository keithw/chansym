#ifndef DELAY_CHANNEL_HPP
#define DELAY_CHANNEL_HPP

#include "channel.hpp"

struct ScheduledPacket {
  double delivery_time;
  Packet packet;

  ScheduledPacket( double s_time, Packet s_pack )
    : delivery_time( s_time ), packet( s_pack )
  {}
};

class DelayChannel : public Channel {
private:
  double delay;
  list<ScheduledPacket> contents;

public:
  DelayChannel( Time *tick, double s_delay );

  void send( Packet pack );
  void wakeup( void );

  CLONEMETHOD(DelayChannel)
};

#endif
