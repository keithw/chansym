#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <stdlib.h>
#include <assert.h>

#include "packet.hpp"

class Container;

class Channel {
protected:
  int addr;
  Container *container;

public:
  Channel() : addr( -1 ), container( NULL ) {}
  virtual ~Channel() {}

  void connect( int s_addr, Container *s_container ) { assert( (addr == -1) || (addr == s_addr) ); addr = s_addr; container = s_container; }

  virtual void init( void ) {} /* Called by container once after connect() */
  virtual void wakeup( void ) {} /* Called by container after prior sleep_until() upcall */
  virtual void send( Packet pack ) = 0; /* Packet input */
  virtual void uncork( void ) {} /* Packet can be sent without loss */
  virtual bool sendable( void ) = 0; /* Can packet be input? */

  Channel( const Channel &x ) : addr( x.addr ), container( NULL ) {}
  Channel & operator=( const Channel &x ) { addr = x.addr; return *this; }
};

#endif
