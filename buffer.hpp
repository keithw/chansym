#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <list>
#include "channel.hpp"

class Buffer : public Channel {
private:
  int size;
  int occupancy;
  std::list<Packet> contents;

public:
  Buffer( int s_size );

  void wakeup( void ) { assert( false ); } /* we don't call sleep_until() */
  bool sendable( void ) { assert( false ); } /* we don't know size of proposed packet */

  void send( Packet pack );
  void uncork( void );
};

#endif
