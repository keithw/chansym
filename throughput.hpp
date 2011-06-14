#ifndef THROUGHPUT_HPP
#define THROUGHPUT_HPP

#include "channel.hpp"

class Throughput : public Channel {
private:
  double throughput; /* bits per second */
  double next_free_time;

  bool is_busy( void );
  bool is_free( void );
  
  Packet stash;

public:
  Throughput( double s_throughput );

  void send( Packet pack );
  bool sendable( void ) { return is_free(); }
  void wakeup( void );
};

#endif
