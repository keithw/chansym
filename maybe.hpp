#ifndef MAYBE_HPP
#define MAYBE_HPP

#include <stdlib.h>
#include <unistd.h>
#include <boost/functional/hash.hpp>

using namespace std;

template <class Contained>
class Maybe {
public:
  Contained *object;

  Maybe() : object( nullptr ) {}
  Maybe( const Contained &x ) : object( new Contained( x ) ) {}

  Maybe( const Maybe &x )
    : object( nullptr )
  {
    if ( x.object ) {
      object = new Contained( *x.object );
    }
  }

  Maybe & operator=( const Maybe &x )
  {
    if ( x.object ) {
      object = new Contained( *x.object );
    }
    return *this;
  }

  bool operator==( const Maybe &x ) const
  {
    if ( ( !object ) && ( !x.object ) ) {
      return true;
    }

    if ( ( !object ) || ( !x.object ) ) {
      return false;
    }

    return ( *object == *(x.object) );
  }

  ~Maybe() { if ( object ) { delete object; } }

  size_t hash( void ) const
  {
    if ( !object ) {
      return 0;
    }
    
    boost::hash<Contained> hasher;
    return hasher( *object );
  }
};

template <class Contained>
size_t hash_value( Maybe<Contained> const &x )
{
  return x.hash();
}

#endif
