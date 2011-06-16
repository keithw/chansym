#ifndef SERIES_HPP
#define SERIES_HPP

#include "channel.hpp"
#include "container.hpp"

template <class First, class Second>
class Series : public Channel, public Container
{
private:
  First a;
  Second b;

public:
  Series( First s_a, Second s_b );
  Series( const Series<First, Second> &x );

  First *get_first( void ) { return &a; }
  Second *get_second( void ) { return &b; }

  /* Channel methods */
  void init( void );
  void wakeup( void );
  void send( Packet pack );
  void uncork( void );
  bool sendable( void );

  /* Container methods */
  void sleep_until( double time, int source_addr ) { wakeups.push( Event( time, source_addr ) ); container->sleep_until( time, addr ); }
  void signal_sendable( int source_addr );
  bool can_send( int source_addr );
  void receive( int source_addr, Packet p );
  double time( void ) { return container->time(); }

  void fork( int source_addr, double my_probability, Channel *other );

  bool operator==( const Series<First, Second> &x ) const { return (a == x.a) && (b == x.b); }
};

template <class First, class Second>
Series<First, Second> series( First a, Second b )
{
  return Series<First, Second>( a, b );
}

#endif
