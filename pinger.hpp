#ifndef PINGER_HPP
#define PINGER_HPP

#include "time.hpp"
#include "channel.hpp"

class Pinger : public Actor
{
private:
  Channel *channel;
  double next_ping_time;
  double increment;
  int counter;

public:  
  Pinger( Time *time, Channel *s_channel, double s_inc );
  void wakeup( void );
};

#endif
