#ifndef PACKET_HPP
#define PACKET_HPP

#include <boost/functional/hash.hpp>

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

  friend size_t hash_value( Packet const & p )
  {
    size_t seed = 0;

    boost::hash_combine( seed, p.length );
    boost::hash_combine( seed, p.src );
    boost::hash_combine( seed, p.id );
    boost::hash_combine( seed, p.send_time );

    return seed;
  }
};

static const Packet DummyPacket( -1, -1, -1, -1 );

struct ScheduledPacket {
  double delivery_time;
  Packet packet;

  ScheduledPacket( double s_time, Packet s_pack )
    : delivery_time( s_time ), packet( s_pack )
  {}

  bool operator==( const ScheduledPacket &x ) const { return (delivery_time == x.delivery_time) && (packet == x.packet); }

  friend size_t hash_value( ScheduledPacket const & p )
  {
    size_t seed = 0;

    boost::hash_combine( seed, p.delivery_time );
    boost::hash_combine( seed, p.packet );

    return seed;
  }
};

#endif
