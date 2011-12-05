#ifndef VALUE_ITERATOR_HPP
#define VALUE_ITERATOR_HPP

#include <list>
#include <vector>
#include "maybe.hpp"
#include "ensemble_container.hpp"

#include <google/sparse_hash_map>

template <class ChannelType>
class Extractor;

template <class ChannelType>
class ValueIterator
{
private:
  std::vector< EnsembleContainer<ChannelType> > exemplar_states;

  Extractor<ChannelType> *extractor;

  int id;

  class WeightedChannel {
  public:
    double probability;
    ChannelType channel;
  
    WeightedChannel( double s_prob, const ChannelType &s_chan ) : probability( s_prob ), channel( s_chan ) {}
  };

  class VIValue {
  public:
    bool initialized;

    size_t exemplar_state_index;

    std::vector<WeightedChannel> quantized_send_indices;
    std::vector<WeightedChannel> quantized_nosend_indices;

    double value;

    VIValue();
  };

  typedef Maybe<ChannelType> key_t;

  typedef google::sparse_hash_map< key_t, VIValue, boost::hash<key_t> > value_map_t;
  value_map_t state_values; /* for Value Iteration */

  std::list<size_t> incomplete_states;

public:
  ValueIterator( Extractor<ChannelType> *s_extractor, int s_id );
  void add_state( const ChannelType &chan );

  void rationalize( void );

  ValueIterator( const ValueIterator &x );
  ValueIterator & operator=( const ValueIterator &x );

  size_t size( void ) const { return state_values.size(); }
};

#endif
