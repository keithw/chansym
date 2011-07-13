#ifndef EXTRACTOR_HPP
#define EXTRACTOR_HPP

#include "collector.hpp"
#include "pawn.hpp"

template <class ChannelType>
class Extractor
{
public:
  virtual Collector & get_collector( ChannelType & ch ) = 0;
  virtual Pawn & get_pawn( ChannelType & ch ) = 0;

  virtual ~Extractor() {}
};

#endif
