#include "value_iterator.hpp"
#include "ensemble_container.hpp"
#include "extractor.hpp"

template <class ChannelType>
ValueIterator<ChannelType>::ValueIterator( Extractor<ChannelType> *s_extractor, int s_id )
  : exemplar_states(),
    extractor( s_extractor ),
    id( s_id ),
    state_values(),
    incomplete_states()
{}

template <class ChannelType>
ValueIterator<ChannelType>::ValueIterator( const ValueIterator &x )
  : exemplar_states( x.exemplar_states ),
    extractor( x.extractor ),
    id( x.id ),
    state_values( x.state_values ),
    incomplete_states( x.incomplete_states )
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
  : initialized( false ),
    exemplar_state_index( -1 ),
    quantized_send_indices(),
    quantized_nosend_indices(),
    value( -INT_MAX )
{}

template <class ChannelType>
void ValueIterator<ChannelType>::add_state( const ChannelType &chan )
{
  /* Step 1: See if we already know the value. */

  Maybe< ChannelType > chanqm( chan );
  chanqm.object->quantize_markovize();

  typename value_map_t::const_iterator it = state_values.find( chanqm );
  if ( it != state_values.end() ) {
    return;
  }

  printf( "Adding QM state %s\n",
	  chanqm.object->identify().c_str() );

  /* Step 2: If not, insert this quantized state (and its unquantized, unrationalized exemplar) into the lists. */

  VIValue new_vi_value;

  EnsembleContainer<ChannelType> exemplar;

  exemplar.advance_to( chan.get_container()->time() );
  exemplar.add_mature( ChannelType( chan ) );

  exemplar_states.push_back( exemplar );
  new_vi_value.exemplar_state_index = exemplar_states.size() - 1;

  state_values[ chanqm ] = new_vi_value;
  incomplete_states.push_back( new_vi_value.exemplar_state_index );
}

template <class ChannelType>
void ValueIterator<ChannelType>::rationalize( void )
{
  /* Step 1: Look for vi_values that haven't been initialized */
  while ( !incomplete_states.empty() ) {
    printf( "Rationalizing (N = %lu)\n", state_values.size() );

    size_t exemplar_index = incomplete_states.front();
    incomplete_states.pop_front();

    EnsembleContainer<ChannelType> &container( exemplar_states[ exemplar_index ] );
    ChannelType &chan( container.get_channel( 0 ).channel );
    double current_time = container.time();
    Maybe< ChannelType > chanqm( chan );
    chanqm.object->quantize_markovize();

    VIValue &vival( state_values[ chanqm ] );

    assert( !vival.initialized );

    /* Step 2: Enumerate the quantized_nosend_indices */
    EnsembleContainer<ChannelType> nosend;

    nosend.advance_to( current_time );
    nosend.add_mature( ChannelType( chan ) );
    nosend.advance_to( current_time + TIME_STEP );

    for ( size_t i = 0; i < nosend.size(); i++ ) {
      extractor->reset( nosend.get_channel( i ).channel );
      ChannelType resultqm( nosend.get_channel( i ).channel );
      resultqm.quantize_markovize();
      WeightedChannel wc( nosend.get_channel( i ).probability, resultqm );
      vival.quantized_nosend_indices.push_back( wc );
    }

    /* Step 3: Enumerate the quantized_send_indices */
    EnsembleContainer<ChannelType> send;
    send.advance_to( current_time );
    send.add_mature( ChannelType( chan ) );
    extractor->get_pawn( send.get_channel( 0 ).channel ).send( Packet( 12000, id, 0, -1 ) );
    send.advance_to( current_time + TIME_STEP );
  
    for ( size_t i = 0; i < send.size(); i++ ) {
      extractor->reset( send.get_channel( i ).channel );
      ChannelType resultqm( send.get_channel( i ).channel );
      resultqm.quantize_markovize();
      WeightedChannel wc( send.get_channel( i ).probability, resultqm );
      vival.quantized_send_indices.push_back( wc );
    }

    vival.initialized = true;

    /* Step 4: Add the states */
    for ( size_t i = 0; i < nosend.size(); i++ ) {
      add_state( nosend.get_channel( i ).channel );
    }

    for ( size_t i = 0; i < send.size(); i++ ) {
      add_state( send.get_channel( i ).channel );
    }
  }
}
