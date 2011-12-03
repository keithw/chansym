#ifndef SQUAREWAVE_HPP
#define SQUAREWAVE_HPP

#include <list>

#include "channel.hpp"
#include "close.hpp"

class SquareWave : public Channel {
private:
  double period;

  bool working( void )
  {
    double current_time = container->time();

    while ( current_time >= period ) {
      current_time -= period;
    }

    return current_time < (period/2);
  }

public:
  SquareWave( double s_period ) : period( s_period ) {}

  void send( Packet pack ) { if ( working() ) { container->receive( addr, pack ); } }
  bool sendable( void ) { return working(); }

  bool operator==( const SquareWave &x ) const { return (period == x.period); }

  string identify( void ) const { return "SquareWave"; }

  void quantize_markovize( void ) { period = quantize_time( period ); }

  friend size_t hash_value( SquareWave const & ch )
  {
    boost::hash<double> hasher;
    return hasher( ch.period );
  }
};

#endif
