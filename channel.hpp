#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <list>
#include <string>

#include "time.hpp"

using namespace std;

class Packet {
public:
  int src;
  int length; /* bits */

  Packet( int s_src, int s_length )
    : src( s_src ), length( s_length )
  {}
};

class Channel : public Actor {
protected:
  Channel *dest;
  Channel *src;

public:
  Channel( Time *tick ) : Actor( tick ), dest( NULL ), src( NULL ) {}

  virtual void send( Packet pack ) = 0;
  virtual void connect( Channel *s_dest ) { dest = s_dest; }
  virtual void register_wakeup( Channel *s_src ) { src = s_src; }

  Channel( const Channel & );
  Channel & operator=( const Channel & );
};

#endif
