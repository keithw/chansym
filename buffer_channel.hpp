#ifndef BUFFER_CHANNEL_HPP
#define BUFFER_CHANNEL_HPP

#include "channel.hpp"

class BufferChannel : public Channel {
private:
  const int size;
  int occupancy;
  list<Packet> contents;

  void try_drain( void );

public:
  BufferChannel( Time *tick, int s_size );

  void send( Packet pack );
  void wakeup( void );
};

#endif
