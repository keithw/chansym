#ifndef COLLECTOR_HPP
#define COLLECTOR_HPP

#include <vector>
#include <boost/functional/hash.hpp>

#include "packet.hpp"
#include "channel.hpp"
#include "container.hpp"

class SignallingCollector;

class Waker
{
public:
  virtual void wakeup_smart_sender( SignallingCollector *ch, double time ) = 0;
  virtual ~Waker() {}
};

using namespace std;

class Collector : public Channel {
protected:
  vector<ScheduledPacket> packets;

public:
  Collector() : packets() {}

  void send( Packet pack ) {
    packets.push_back( ScheduledPacket( container->time(), pack ) ); container->receive( addr, pack );
  }

  bool sendable( void ) { return true; }

  vector<ScheduledPacket> & get_packets( void ) { return packets; }
  void reset( void ) { packets.clear(); }

  bool operator==( const Collector &x ) const { return packets == x.packets; }

  string identify( void ) const { return "Collector"; }

  friend size_t hash_value( Collector const & ch )
  {
    boost::hash<vector<ScheduledPacket>> hasher;
    return hasher( ch.packets );
  }
};

class SignallingCollector : public Collector {
private:
  Waker *waker;

public:
  SignallingCollector( Waker *s_waker ) : waker( s_waker ) {}
  SignallingCollector & operator=( const SignallingCollector &x )
  {
    Collector::operator=( x );
    waker = x.waker;
    return *this;
  }

  SignallingCollector( const SignallingCollector &x )
    : Collector( x ), waker( x.waker )
  {}

  void send( Packet pack ) {
    packets.push_back( ScheduledPacket( container->time(), pack ) );
    container->receive( addr, pack );
    waker->wakeup_smart_sender( this, container->time() );
  }
};

#endif
