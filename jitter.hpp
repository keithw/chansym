#ifndef JITTER_HPP
#define JITTER_HPP

#include <list>

#include "channel.hpp"

class Jitter : public Channel {
private:
  double delay;
  double delay_probability;
  std::list<ScheduledPacket> contents;

public:
  class ForkState : public Channel::ForkState {
  public:
    Packet pack;
    ForkState( Packet s_pack ) : pack( s_pack ) {}
    ForkState( void ) : pack( -1, -1, -1, -1 ) {}
  };

  Jitter( double s_delay, double s_delay_probability );

  void send( Packet pack );
  bool sendable( void ) { return true; }
  void wakeup( void );

  void after_fork( bool is_other, ForkState x );

  bool operator==( const Jitter &x ) const { return (delay == x.delay)
      && (delay_probability == x.delay_probability) && (contents == x.contents); }

  string identify( void );

  friend size_t hash_value( Jitter const & ch );
};

#endif
