#ifndef EMBEDDABLE_ENSEMBLE_HPP
#define EMBEDDABLE_ENSEMBLE_HPP

#include "channel.hpp"
#include "ensemble_container.hpp"

template <class ChannelType>
class EmbeddableEnsemble : public Channel, public EnsembleContainer<ChannelType> {
public:
  EmbeddableEnsemble( EnsembleContainer<ChannelType> const &x )
    : EnsembleContainer<ChannelType>( x )
  {}

  void sleep_until( double time, int source_addr, int sort_order=0 )
  {
    EnsembleContainer<ChannelType>::wakeups.push( Event( time, source_addr, sort_order ) );
    container->sleep_until( time, addr, sort_order );
  }

  void wakeup( void )
  {
    assert( !EnsembleContainer<ChannelType>::wakeups.empty() );

    Event next_event = EnsembleContainer<ChannelType>::wakeups.top();
    assert( next_event.time == container->time() );
    EnsembleContainer<ChannelType>::the_time = next_event.time;

    EnsembleContainer<ChannelType>::tick();
  }

  void advance_to( double ) { assert( false ); }

  void send( Packet ) { assert( false ); } 
  bool sendable( void ) { assert( false ); }

  string identify( void )
  {
    return EnsembleContainer<ChannelType>::identify();
  }

  friend size_t hash_value( EmbeddableEnsemble const &x )
  {
    return x.hash();
  }
};

#endif
