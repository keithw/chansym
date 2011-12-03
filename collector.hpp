#ifndef COLLECTOR_HPP
#define COLLECTOR_HPP

#include <sstream>
#include <vector>
#include <boost/functional/hash.hpp>
#include <algorithm>

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

  void quantize_markovize( void ) {
    double now = container->time();

    for_each( packets.begin(), packets.end(),
	      [now]( ScheduledPacket &x ) { x.delivery_time = quantize_time( x.delivery_time - now );
		x.packet.length = quantize_length( x.packet.length );
		x.packet.send_time = quantize_time( x.packet.send_time - now ); } );
  }

  vector<ScheduledPacket> & get_packets( void ) { return packets; }
  void reset( void ) { packets.clear(); }

  bool operator==( const Collector &x ) const { return packets == x.packets; }

  string identify( void ) const {
      ostringstream response;
      response << "Collector( ";
      response << packets.size();

      for ( vector<ScheduledPacket>::const_iterator i = packets.begin();
	    i != packets.end();
	    i++ ) {
	response << ", < ";
	response << i->packet.src;
	response << ", ";
	response << i->packet.id;
	response << ", ";
	response << i->packet.send_time;
	response << " to ";
	response << i->delivery_time;
	response << " >";
      }

      response << " )";
      
      return response.str();
  }
  
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
