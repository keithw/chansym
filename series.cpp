#include "series.hpp"

template <class First, class Second>
Series<First, Second>::Series( First s_a, Second s_b )
  : a( s_a ), b( s_b )
{
  a.connect( 0, this );
  b.connect( 1, this );
}

template <class First, class Second>
Series<First, Second>::Series( const Series<First, Second> &x )
  : Channel( x ), Container( x ), a( x.a ), b( x.b )
{
  a.connect( 0, this );
  b.connect( 1, this );
}

template <class First, class Second>
void Series<First, Second>::init( void )
{
  a.init();
  b.init();
}

template <class First, class Second>
void Series<First, Second>::wakeup( void )
{
  assert( !wakeups.empty() );
  
  Event next_event = wakeups.top();

  assert( next_event.time == container->time() );

  wakeups.pop();

  switch ( addr ) {
  case 0:
    a.wakeup();
    break;
  case 1:
    b.wakeup();
    break;
  default:
    assert( false );
  }
}

template <class First, class Second>
void Series<First, Second>::send( Packet pack )
{
  a.send( pack );
}

template <class First, class Second>
void Series<First, Second>::uncork( void )
{
  b.uncork();
}

template <class First, class Second>
bool Series<First, Second>::sendable( void )
{
  return a.sendable();
}

template <class First, class Second>
void Series<First, Second>::signal_sendable( int source_addr )
{
  assert( container );

  switch ( source_addr ) {
  case 0:
    container->signal_sendable( addr );
    break;
  case 1:
    a.uncork();
    break;
  default:
    assert( false );
  }
}

template <class First, class Second>
bool Series<First, Second>::can_send( int source_addr )
{
  assert( container );

  switch ( source_addr ) {
  case 0:
    return b.sendable();
  case 1:
    return container->can_send( addr );
  default:
    assert( false );
  }
}

template <class First, class Second>
void Series<First, Second>::receive( int source_addr, Packet p )
{
  assert( container );

  switch( source_addr ) {
  case 0:
    b.send( p );
    break;
  case 1:
    container->receive( addr, p );
    break;
  default:
    assert( false );
  }
}
