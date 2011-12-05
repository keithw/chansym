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
  class ForkState : public Channel::ForkState {
  public:
    typename First::ForkState a;
    typename Second::ForkState b;

    ForkState( typename First::ForkState s_a, typename Second::ForkState s_b ) : a( s_a ), b( s_b ) {}
    ForkState( void ) : a(), b() {}
  };

  Series( First s_a, Second s_b );
  Series( const Series<First, Second> &x );
  Series<First, Second> & operator=( const Series<First, Second> &x );

  First &get_first( void ) { return a; }
  Second &get_second( void ) { return b; }

  /* Channel methods */
  void init( void );
  void wakeup( void );
  void send( Packet pack );
  void uncork( void );
  bool sendable( void );

  void after_fork( bool is_other, ForkState x );

  void quantize_markovize( void ) { a.quantize_markovize(); b.quantize_markovize(); rationalize_wakeups(); }

  /* Container methods */
  void sleep_until( double time, int source_addr, int sort_order=0 ) { wakeups.push( Event( time, source_addr, sort_order ) ); container->sleep_until( time, addr, sort_order ); }
  void signal_sendable( int source_addr );
  bool can_send( int source_addr );
  void receive( int source_addr, Packet p );
  double time( void ) const { return container->time(); }

  void fork( int source_addr, double my_probability, Channel::ForkState *fs );
  double probability( int ) { return container->probability( addr ); }

  bool operator==( const Series<First, Second> &x ) const { return (a == x.a) && (b == x.b) && (wakeups == x.wakeups); }

  string identify( void ) const;

  size_t hash( void ) const;

  void clear_wakeups( void ) { assert( false ); }
  void clear_wakeups( int ) { assert( false ); }
};

template <class First, class Second>
size_t hash_value( Series<First, Second> const &x )
{
  return x.hash();
}

template <class First, class Second>
Series<First, Second> series( First a, Second b )
{
  return Series<First, Second>( a, b );
}

#endif
