#ifndef PAWN_HPP
#define PAWN_HPP

#include "channel.hpp"

class Pawn : public Channel
{
public:
  Pawn( void ) {}

  void send( Packet pack ) { container->receive( addr, pack ); }
  void uncork( void ) { container->signal_sendable( addr ); }
  bool sendable( void ) { return container->can_send( addr ); }

  bool operator==( const Pawn & ) const { return true; }

  string identify( void ) { return "Pawn"; }

  friend size_t hash_value( Pawn const & ) { return 0; }
};

#endif
