#ifndef ISENDER_HPP
#define ISENDER_HPP

#include "channel.hpp"
#include "collector.hpp"
#include "ensemble_container.hpp"
#include "extractor.hpp"

template <class ChannelType>
class ISender : public Channel
{
private:
  EnsembleContainer<ChannelType> prior;
  Extractor<ChannelType> *extractor;
  Collector *collector;

  double next_ping_time, increment;
  int counter, id;
  unsigned int smallestsize;

  void sendout( Packet p );

public:  
  ISender( EnsembleContainer<ChannelType> s_prior,
	   Extractor<ChannelType> *s_extractor );

  void init( void );
  void wakeup( void );
  void send( Packet pack ) { container->receive( addr, pack ); }
  void uncork( void ) { container->signal_sendable( addr ); }
  bool sendable( void ) { return container->can_send( addr ); }

  void set_collector( Collector &x ) { collector = &x; }

  bool operator==( const ISender<ChannelType> &x ) const { return (prior == x.prior); }

  string identify( void ) { return "ISender"; }

  size_t hash( void ) const;

  ISender<ChannelType> & operator=( const ISender<ChannelType> &x );
  ISender( const ISender<ChannelType> & );
};

template <class ChannelType>
size_t hash_value( ISender<ChannelType> const &x )
{
  return x.hash();
}

#endif
