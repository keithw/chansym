#ifndef THROUGHPUT_CHANNEL_HPP
#define THROUGHPUT_CHANNEL_HPP

#include "channel.hpp"

class ThroughputChannel : public Channel {
private:
  double throughput; /* bits per second */
  double next_free_time;

  bool is_busy( void ) { return next_free_time > time->now(); };
  bool is_free( void ) { return !is_busy(); }
  
public:
  ThroughputChannel( Time *tick, double s_throughput );

  void send( Packet pack );
  bool sendable( void ) { return is_free(); }
  void wakeup( void );

  CLONEMETHOD(ThroughputChannel)
};

#endif
