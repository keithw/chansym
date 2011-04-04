#ifndef SERIES_CHANNEL_HPP
#define SERIES_CHANNEL_HPP

#include "channel.hpp"

class SeriesChannel : public Channel {
private:
  Channel *a, *b;

public:
  SeriesChannel( Time *tick, Channel *s_a, Channel *s_b );
  ~SeriesChannel();

  void send( Packet pack ) { a->send( pack ); }
  bool sendable( void ) { return a->sendable(); }
  void register_wakeup( Channel *s_src ) { return a->register_wakeup( s_src ); }
  void connect( Channel *s_dest ) { b->connect( s_dest ); }
  void wakeup( void ) { b->wakeup(); }

  SeriesChannel( const SeriesChannel & );
  SeriesChannel & operator=( const SeriesChannel & );
};

#endif
