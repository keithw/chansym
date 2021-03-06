#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <stdlib.h>
#include <assert.h>
#include <string>

#include "packet.hpp"

using namespace std;

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

  void set_forking( void ) { assert( !forking ); forking = true; }

  virtual void init( void ) {} /* Called by container once after connect() */
  virtual void wakeup( void ) {} /* Called by container after prior sleep_until() upcall */
  virtual void send( Packet pack ) = 0; /* Packet input */
  virtual void uncork( void ) {} /* Packet can be sent without loss */
  virtual bool sendable( void ) = 0; /* Can packet be input? */

  virtual void after_fork( bool, ForkState ) { forking = false; }

  Channel( const Channel &x ) : forking( x.forking ), addr( x.addr ), container( x.container ) {}
  Channel & operator=( const Channel &x ) { forking = x.forking; addr = x.addr; container = x.container; return *this; }

  virtual std::string identify( void ) const { return "[Unidentified Channel]"; }

  friend size_t hash_value( Channel const & ) { return 0; }

  const Container * get_container( void ) const { assert( container ); return container; }
  Channel * get_container_channel( void );


  virtual void quantize_markovize( void ) = 0;
};

#endif
