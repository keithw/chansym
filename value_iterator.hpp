#ifndef VALUE_ITERATOR_HPP
#define VALUE_ITERATOR_HPP

#include <vector>
#include "maybe.hpp"

#include <google/dense_hash_map>

template <class ChannelType>
class ValueIterator
{
private:
  std::vector<ChannelType> quantized_states;
  std::vector<ChannelType> exemplar_states;

  class WeightedChannel {
  public:
    double probability;
    size_t quantized_index;
  };

  class VIValue {
  public:
    size_t exemplar_state_index;

    std::vector<WeightedChannel> quantized_send_indices;
    std::vector<WeightedChannel> quantized_nosend_indices;

    double value;

    VIValue();
  };

  class boost_hasher {
  public:
    size_t operator() ( const Maybe<ChannelType> &x ) const
    {
      return hash_value( x );
    }
  };

  typedef Maybe<ChannelType> key_t;

  typedef google::dense_hash_map< key_t, VIValue, boost_hasher > value_map_t;
  value_map_t state_values; /* for Value Iteration */

public:
  ValueIterator();
  double value( const ChannelType &chan );
};

#endif
