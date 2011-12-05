#include "value_iterator.hpp"
#include "ensemble_container.hpp"
#include "extractor.hpp"

template <class ChannelType>
ValueIterator<ChannelType>::ValueIterator( Extractor<ChannelType> *s_extractor, int s_id )
  : exemplar_states(),
    extractor( s_extractor ),
    id( s_id ),
    state_values()
{}

template <class ChannelType>
ValueIterator<ChannelType>::ValueIterator( const ValueIterator &x )
  : exemplar_states( x.exemplar_states ),
    extractor( x.extractor ),
    id( x.id ),
    state_values( x.state_values )
{}

template <class ChannelType>
ValueIterator<ChannelType> & ValueIterator<ChannelType>::operator=( const ValueIterator &x )
{
  exemplar_states = x.exemplar_states;
  extractor = x.extractor;
  id = x.id;
  state_values = x.state_values;

  return *this;
}

template <class ChannelType>
ValueIterator<ChannelType>::VIValue::VIValue()
  : exemplar_state_index( -1 ),
    quantized_send_indices(),
    quantized_nosend_indices(),
    value( 0 )
{}

template <class ChannelType>
void ValueIterator<ChannelType>::add_state( const ChannelType &chan )
{
  /* Step 1: See if we already know the value. */

  Maybe< ChannelType > chanqm( chan );
  chanqm.object->quantize_markovize();

  printf( "hashval = %lu\n", hash_value( chanqm ) );

  typename value_map_t::const_iterator it = state_values.find( chanqm );
  if ( it != state_values.end() ) {
    printf( "Already had state!!!!\n" );
    return;
  }

  /* Step 2: If not, insert this quantized state (and its unquantized, unrationalized exemplar) into the lists. */
  VIValue new_vi_value;
  exemplar_states.push_back( chan );
  new_vi_value.exemplar_state_index = exemplar_states.size() - 1;
  
  /* Step 3: Enumerate the quantized_nosend_indices */
  EnsembleContainer<ChannelType> nosend;
  double current_time = chan.time();

  printf( "current_time = %f\n", current_time );

  nosend.advance_to( current_time );
  nosend.add_mature( ChannelType( chan ) );
  nosend.advance_to( current_time + TIME_STEP );

  for ( size_t i = 0; i < nosend.size(); i++ ) {
    extractor->reset( nosend.get_channel( i ).channel );
    ChannelType resultqm( nosend.get_channel( i ).channel );
    resultqm.quantize_markovize();
    WeightedChannel wc( nosend.get_channel( i ).probability, resultqm );
    new_vi_value.quantized_nosend_indices.push_back( wc );
  }

  /* Step 4: Enumerate the quantized_send_indices */
  EnsembleContainer<ChannelType> send;
  send.advance_to( current_time );
  send.add_mature( ChannelType( chan ) );
  extractor->get_pawn( send.get_channel( 0 ).channel ).send( Packet( 12000, id, 0, current_time ) );
  send.advance_to( current_time + TIME_STEP );
  
  for ( size_t i = 0; i < send.size(); i++ ) {
    extractor->reset( send.get_channel( i ).channel );
    ChannelType resultqm( send.get_channel( i ).channel );
    resultqm.quantize_markovize();
    WeightedChannel wc( send.get_channel( i ).probability, resultqm );
    new_vi_value.quantized_send_indices.push_back( wc );
  }

  /* Step 5: Add the state */
  new_vi_value.value = -INT_MAX;

  state_values[ chanqm ] = new_vi_value;

  printf( "Added state %s, state_values.size() now %lu\n",
	  chanqm.object->identify().c_str(),
	  state_values.size() );

  /* Recurse */
  for ( size_t i = 0; i < nosend.size(); i++ ) {
    add_state( nosend.get_channel( i ).channel );
  }
}
