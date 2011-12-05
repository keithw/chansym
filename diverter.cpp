#include <boost/functional/hash.hpp>

#include "diverter.hpp"

template <class First, class Second>
Diverter<First, Second>::Diverter( First s_a, Second s_b )
  : a( s_a ), b( s_b )
{
  a.connect( 0, this );
  b.connect( 1, this );
}

template <class First, class Second>
Diverter<First, Second>::Diverter( const Diverter<First, Second> &x )
  : Channel( x ), Container( x ), a( x.a ), b( x.b )
{
  a.connect( 0, this );
  b.connect( 1, this );
}

template <class First, class Second>
Diverter<First, Second> & Diverter<First, Second>::operator=( const Diverter<First, Second> &x )
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
void Diverter<First, Second>::init( void )
{
  a.init();
  b.init();
}

template <class First, class Second>
void Diverter<First, Second>::wakeup( void )
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
void Diverter<First, Second>::send( Packet pack )
{
  if ( pack.src == 0 ) {
    a.send( pack );
  } else if ( pack.src == 1 ) {
    b.send( pack );
  }
}

template <class First, class Second>
void Diverter<First, Second>::uncork( void )
{
  assert( false );
}

template <class First, class Second>
bool Diverter<First, Second>::sendable( void )
{
  assert( false );
}

template <class First, class Second>
void Diverter<First, Second>::after_fork( bool is_other, ForkState x )
{
  forking = false;

  a.after_fork( is_other, x.a );
  b.after_fork( is_other, x.b );
}

template <class First, class Second>
void Diverter<First, Second>::signal_sendable( int )
{
  assert( false );
}

template <class First, class Second>
bool Diverter<First, Second>::can_send( int )
{
  return container->can_send( addr );
}

template <class First, class Second>
void Diverter<First, Second>::receive( int, Packet p )
{
  container->receive( addr, p );
}

template <class First, class Second>
void Diverter<First, Second>::fork( int source_addr, double my_probability, Channel::ForkState *fs )
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
    return;
  }

  delete fs;
  container->fork( addr, my_probability, new_fs );
}

template <class First, class Second>
string Diverter<First, Second>::identify( void ) const
{
  return "< " + a.identify() + " > | < " + b.identify() + " >";
}

template <class First, class Second>
size_t Diverter<First, Second>::hash( void ) const
{
  size_t seed = 0;

  boost::hash_combine( seed, a );
  boost::hash_combine( seed, b );
  boost::hash_combine( seed, round_wakeups( wakeups ) );

  return seed;
}
