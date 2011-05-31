#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <list>
#include <string>
#include <assert.h>

#include "time.hpp"

using namespace std;

#define CLONEMETHOD(TypeName) TypeName *clone( void ) { TypeName *x = new TypeName( *this ); return x; } void assign( const Channel *x ) { const TypeName *x_oftype = dynamic_cast<const TypeName *>( x ); assert( x_oftype ); *this = *x_oftype; }

class Packet {
public:
  int length; /* bits */

  int src, id;
  double send_time;

  Packet( int s_length, int s_src, int s_id, double s_send_time )
    : length( s_length ), src( s_src ), id( s_id ), send_time( s_send_time )
  {}
};

class Channel : public Actor {
protected:
  Channel *dest;
  Channel *src;

public:
  Channel( Time *tick ) : Actor( tick ), dest( NULL ), src( NULL ) {}

  virtual void send( Packet pack ) = 0;
  virtual bool sendable( void ) { return true; }
  virtual void register_wakeup( Channel *s_src ) { src = s_src; }
  virtual void connect( Channel *s_dest )
  {
    dest = s_dest;
    dest->register_wakeup( this );
  }

  virtual ~Channel() {}

  virtual Channel *clone( void ) = 0;
  virtual void assign( const Channel *x ) = 0;

  Channel( const Channel & ) : Actor( NULL ), dest( NULL ), src( NULL ) {}

  Channel & operator=( const Channel & ) { return *this; }
};

#endif
