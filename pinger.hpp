#ifndef PINGER_HPP
#define PINGER_HPP

#include <assert.h>

#include "time.hpp"
#include "channel.hpp"

class Pinger : public Channel
{
private:
  double next_ping_time;
  double increment;
  int counter;

public:  
  Pinger( Time *time, double s_inc );
  void wakeup( void );
  void send( Packet ) { assert( false ); }
};

#endif
