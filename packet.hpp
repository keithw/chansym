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

  bool operator==( const Packet &x ) const
  {
    return (length == x.length) && (src == x.src) && (id == x.id) && (send_time == x.send_time);
  }
};

struct ScheduledPacket {
  double delivery_time;
  Packet packet;

  ScheduledPacket( double s_time, Packet s_pack )
    : delivery_time( s_time ), packet( s_pack )
  {}

  bool operator==( const ScheduledPacket &x ) const { return (delivery_time == x.delivery_time) && (packet == x.packet); }
};

#endif
