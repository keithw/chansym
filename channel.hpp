#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <list>
#include <string>

#include "time.hpp"

using namespace std;

class Packet {
public:
  int src;
  int length;

  Packet( int s_src, int s_length )
    : src( s_src ), length( s_length )
  {}
};

class Channel : public Actor {
public:
  Channel( Time *tick ) : Actor( tick ) {}

  virtual void send( Packet pack ) = 0;
  virtual void connect( Channel *s_dest ) = 0;
};

#endif
