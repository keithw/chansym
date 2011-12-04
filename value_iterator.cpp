#include "value_iterator.hpp"
#include "ensemble_container.hpp"

template <class ChannelType>
ValueIterator<ChannelType>::ValueIterator()
  : quantized_states(),
    exemplar_states(),
    state_values()
{}

template <class ChannelType>
ValueIterator<ChannelType>::VIValue::VIValue()
  : exemplar_state_index( -1 ),
    quantized_send_indices(),
    quantized_nosend_indices(),
    value( 0 )
{}

template <class ChannelType>
double ValueIterator<ChannelType>::value( const ChannelType &chan )
{
  /* Step 1: See if we already know the value. */

  Maybe< ChannelType > chanqm( chan );
  chanqm.object->quantize_markovize();
  typename value_map_t::const_iterator it = state_values.find( chanqm );
  if ( it != state_values.end() ) {
    return state_values[ chanqm ].value;
  }

  /* Step 2: If not, insert this quantized state (and its unquantized, unrationalized exemplar) into the lists. */
  quantized_states.push_back( *chanqm.object );
  VIValue new_vi_value;
  exemplar_states.push_back( chan );
  new_vi_value.exemplar_state_index = exemplar_states.size() - 1;
  
  /* Step 3: Enumerate the quantized_nosend_indices */
  EnsembleContainer<ChannelType> nosend;
  double current_time = chan.time();
  nosend.advance_to( current_time );
  //  nosend.add_mature( chan );

  return 0.0;
}
