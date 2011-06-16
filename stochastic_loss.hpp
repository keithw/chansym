#ifndef STOCHASTIC_LOSS_HPP
#define STOCHASTIC_LOSS_HPP

#include "channel.hpp"
#include "packet.hpp"

class StochasticLoss : public Channel {
private:
  double loss_probability;

public:
  StochasticLoss( double s_prob );

  void send( Packet pack );
  bool sendable( void ) { return true; }

  bool operator==( const StochasticLoss &x ) const { return (loss_probability == x.loss_probability); }
};

#endif
