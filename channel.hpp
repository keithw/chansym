#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <list>
#include <string>

#include "time.hpp"

using namespace std;

class Packet {
public:
  double delivery_time;
  string payload;

  Packet( double s_time, string s_payload )
    : delivery_time( s_time ), payload( s_payload )
  {}
};

class Channel : public Actor {

private:
  double delay;
  list<Packet> contents;

public:
  void send( string payload );
  void wakeup( void );

  Channel( Time *s_time, double s_delay )
    : Actor( s_time ),
      delay( s_delay ),
      contents() {}
};

#endif
