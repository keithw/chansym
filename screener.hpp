#ifndef SCREENER_HPP
#define SCREENER_HPP

#include "channel.hpp"

class Screener : public Channel {
private:
  int passthrough_src;

public:
  Screener( int s_passthrough_src ) : passthrough_src( s_passthrough_src ) {}

  void send( Packet pack )
  {
    if ( pack.src == passthrough_src ) {
      container->receive( addr, pack );
    }
  }

  bool sendable( void ) { return true; }

  bool operator==( const Screener &x ) const { return passthrough_src == x.passthrough_src; }

  string identify( void ) const { return "Screener"; }

  friend size_t hash_value( Screener const & ch )
  {
    boost::hash<int> hasher;
    return hasher( ch.passthrough_src );
  }
};

#endif
