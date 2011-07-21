#ifndef INTERMITTENT_HPP
#define INTERMITTENT_HPP

#include <list>

#include "channel.hpp"

class Intermittent : public Channel {
private:
  bool working;
  double probability;
  double interval;

public:
  class ForkState : public Channel::ForkState {
  public:
    ForkState( void ) {}
  };

  Intermittent( double s_probability, double s_interval );

  void init( void );
  void wakeup( void );

  void send( Packet pack );
  bool sendable( void ) { return working; }

  void after_fork( bool is_other, ForkState x );

  bool operator==( const Intermittent &x ) const { return (working == x.working)
      && (probability == x.probability) && (interval == x.interval); }

  string identify( void ) const;

  friend size_t hash_value( Intermittent const & ch );
};

#endif
