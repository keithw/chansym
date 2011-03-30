#ifndef STATS_CHANNEL_HPP
#define STATS_CHANNEL_HPP

#include "channel.hpp"

class StatsChannel : public Channel {
private:
  int total_bits;
  int total_delay;

public:
  StatsChannel( Time *tick );

  void send( Packet pack );
  void wakeup( void ) {}

  double average_bps( void );
  double average_delay( void );
};

#endif
