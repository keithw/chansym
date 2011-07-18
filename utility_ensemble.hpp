#ifndef UTILITY_ENSEMBLE_HPP
#define UTILITY_ENSEMBLE_HPP

#include "channel.hpp"
#include "ensemble_container.hpp"

template <class ChannelType>
class UtilityEnsemble : public EnsembleContainer< ChannelType > {
  typedef EnsembleContainer< ChannelType > Parent;

public:
  UtilityEnsemble( double s_time )
    : Parent( s_time )
  {}

  string identify( void ) const;

  void combine( void ) { assert( false ); }
};

#endif
