#ifndef THROUGHPUT_HPP
#define THROUGHPUT_HPP

#include "channel.hpp"

class Throughput : public Channel {
private:
  double throughput; /* bits per second */
  double next_free_time;

  bool is_busy( void ) const { return busy; }
  bool is_free( void ) const { return !busy; }
  
  Packet stash;
  bool busy;

public:
  Throughput( double s_throughput );

  void send( Packet pack );
  bool sendable( void ) { return is_free(); }
  void wakeup( void );

  string identify( void ) const;

  bool operator==( const Throughput &x ) const
  {
    return (throughput == x.throughput) && ( rounder(next_free_time) == rounder(x.next_free_time) )
      && (stash == x.stash) && (busy == x.busy);
  }

  friend size_t hash_value( Throughput const & ch );

  void quantize_markovize( void );
};

#endif
