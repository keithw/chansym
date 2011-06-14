#ifndef PACKET_HPP
#define PACKET_HPP

class Packet {
public:
  int length; /* bits */

  int src, id;
  double send_time;

  Packet( int s_length, int s_src, int s_id, double s_send_time )
    : length( s_length ), src( s_src ), id( s_id ), send_time( s_send_time )
  {}
};

#endif
