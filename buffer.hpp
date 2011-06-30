#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <list>
#include "channel.hpp"

class Buffer : public Channel {
private:
  int size;
  int occupancy;
  std::list<Packet> contents;

  int tmp_initial_num, tmp_initial_size;

public:
  Buffer( int s_size, int initial_num = 0, int initial_size = 0 );

  void init( void );

  void wakeup( void ) { assert( false ); } /* we don't call sleep_until() */
  bool sendable( void ) { assert( false ); } /* we don't know size of proposed packet */

  void send( Packet pack );
  void uncork( void );

  bool operator==( const Buffer &x ) const { return (size == x.size) && (occupancy == x.occupancy) && (contents == x.contents); }

  string identify( void );

  friend size_t hash_value( Buffer const & ch );
};

#endif
