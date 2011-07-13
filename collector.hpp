#ifndef COLLECTOR_HPP
#define COLLECTOR_HPP

#include <vector>
#include <boost/functional/hash.hpp>

#include "packet.hpp"
#include "channel.hpp"
#include "container.hpp"

using namespace std;

class Collector : public Channel {
private:
  vector<ScheduledPacket> packets;

public:
  Collector() : packets() {}

  void send( Packet pack ) { packets.push_back( ScheduledPacket( container->time(), pack ) ); container->receive( addr, pack ); }
  bool sendable( void ) { return true; }

  vector<ScheduledPacket> & get_packets( void ) { return packets; }
  void reset( void ) { packets.clear(); }

  bool operator==( const Collector &x ) const { return packets == x.packets; }

  string identify( void ) { return "Collector"; }

  friend size_t hash_value( Collector const & ch )
  {
    boost::hash<vector<ScheduledPacket>> hasher;
    return hasher( ch.packets );
  }
};

#endif
