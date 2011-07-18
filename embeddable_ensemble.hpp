#ifndef EMBEDDABLE_ENSEMBLE_HPP
#define EMBEDDABLE_ENSEMBLE_HPP

#include "channel.hpp"
#include "ensemble_container.hpp"

template <class ChannelType>
class EmbeddableEnsemble : public Channel, public EnsembleContainer<ChannelType> {
  typedef EnsembleContainer<ChannelType> Parent;

private:
public:
  EmbeddableEnsemble( const Parent &x )
    : Parent( x )
  {}

  double time( void ) { return container->time(); }

  void sleep_until( double time, int source_addr, int sort_order=0 )
  {
    Parent::wakeups.push( Event( time, source_addr, sort_order ) );
    container->sleep_until( time, addr, sort_order );
  }

  void wakeup( void )
  {
    assert( !Parent::wakeups.empty() );

    Event next_event = Parent::wakeups.top();
    assert( next_event.time == container->time() );
    Parent::the_time = next_event.time;

    Parent::tick();
  }

  void advance_to( double ) { assert( false ); }

  void send( Packet ) { assert( false ); } 
  bool sendable( void ) { assert( false ); }

  string identify( void );

  friend size_t hash_value( EmbeddableEnsemble<ChannelType> const &x )
  {
    return x.hash();
  }
};

#endif
