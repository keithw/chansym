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
Series<First, Second> & Series<First, Second>::operator=( const Series<First, Second> &x )
{
  Channel::operator=( x );
  Container::operator=( x );

  a = x.a;
  b = x.b;

  a.connect( 0, this );
  b.connect( 1, this );

  return *this;
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

  assert( container );
  assert( next_event.time == container->time() );

  wakeups.pop();

  switch ( next_event.addr ) {
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
void Series<First, Second>::after_fork( bool is_other, ForkState x )
{
  forking = false;

  a.after_fork( is_other, x.a );
  b.after_fork( is_other, x.b );
}

template <class First, class Second>
void Series<First, Second>::signal_sendable( int source_addr )
{
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

template <class First, class Second>
void Series<First, Second>::fork( int source_addr, double my_probability, Channel::ForkState *fs )
{
  ForkState *new_fs;

  typename First::ForkState *first_fs;
  typename Second::ForkState *second_fs;

  switch( source_addr ) {
  case 0:
    a.set_forking();
    first_fs = dynamic_cast<typename First::ForkState *>( fs );
    assert( first_fs );
    new_fs = new ForkState( *first_fs, typename Second::ForkState() );
    break;
  case 1:
    b.set_forking();
    second_fs = dynamic_cast<typename Second::ForkState *>( fs );
    assert( second_fs );
    new_fs = new ForkState( typename First::ForkState(), *second_fs );
    break;
  default:
    assert( false );
  }

  delete fs;
  container->fork( addr, my_probability, new_fs );
}

template <class First, class Second>
string Series<First, Second>::identify( void )
{
  return "< " + a.identify() + " > => < " + b.identify() + " >";
}
