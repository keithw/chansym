#ifndef UTILITY_ENSEMBLE_HPP
#define UTILITY_ENSEMBLE_HPP

#include "channel.hpp"
#include "ensemble_container.hpp"

template <class ChannelType>
class UtilityEnsemble : public EnsembleContainer< ChannelType > {
  typedef EnsembleContainer< ChannelType > Parent;

public:
  UtilityEnsemble()
    : Parent()
  {}

  string identify( void );

  void combine( void ) { assert( false ); }
};

#endif
