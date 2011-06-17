#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <stdlib.h>
#include <assert.h>

#include "packet.hpp"

class Container;

class Channel {
protected:
  bool forking;
  int addr;
  Container *container;

public:
  class ForkState { public: virtual ~ForkState() {}; };

  Channel() : forking( false ), addr( -1 ), container( NULL ) {}
  virtual ~Channel() {}

  void connect( int s_addr, Container *s_container ) { assert( (addr == -1) || (addr == s_addr) ); addr = s_addr; container = s_container; }
  void newaddr( int s_addr, Container *s_container ) { addr = s_addr; container = s_container; }

  bool get_forking( void ) { return forking; }
  void set_forking( void ) { forking = true; }

  virtual void init( void ) {} /* Called by container once after connect() */
  virtual void wakeup( void ) {} /* Called by container after prior sleep_until() upcall */
  virtual void send( Packet pack ) = 0; /* Packet input */
  virtual void uncork( void ) {} /* Packet can be sent without loss */
  virtual bool sendable( void ) = 0; /* Can packet be input? */

  void after_fork( bool, ForkState ) { forking = false; }

  Channel( const Channel &x ) : forking( x.forking ), addr( x.addr ), container( x.container ) {}
  Channel & operator=( const Channel &x ) { forking = x.forking; addr = x.addr; container = x.container; return *this; }
};

#endif
