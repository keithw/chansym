#ifndef STOCHASTIC_LOSS_HPP
#define STOCHASTIC_LOSS_HPP

#include "channel.hpp"
#include "packet.hpp"

class StochasticLoss : public Channel {
private:
  double loss_probability;

public:
  class ForkState : public Channel::ForkState {
  public:
    Packet pack;
    ForkState( Packet s_pack ) : pack( s_pack ) {}
    ForkState( void ) : pack( -1, -1, -1, -1 ) {}
  };

  StochasticLoss( double s_prob );

  void send( Packet pack );
  bool sendable( void ) { return true; }

  void after_fork( bool is_other, ForkState x );

  string identify( void );

  bool operator==( const StochasticLoss &x ) const { return (loss_probability == x.loss_probability); }

  friend size_t hash_value( StochasticLoss const & ch );
};

#endif
