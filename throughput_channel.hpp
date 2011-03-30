#ifndef THROUGHPUT_CHANNEL_HPP
#define THROUGHPUT_CHANNEL_HPP

#include "channel.hpp"

class ThroughputChannel : public Channel {
private:
  double throughput; /* bits per second */
  double next_free_time;

  bool busy( void ) { return next_free_time > time->now(); };
  bool free( void ) { return !busy(); }
  
public:
  ThroughputChannel( Time *tick, double s_throughput );

  void send( Packet pack );
  void wakeup( void );
};

#endif
