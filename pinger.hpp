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
  string message;

public:  
  Pinger( Time *time, Channel *s_channel, double s_inc, string s_msg );
  void wakeup( void );

  Pinger( const Pinger & );
  Pinger & operator=( const Pinger & );
};

#endif
