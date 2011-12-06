#include "value_iterator.hpp"
#include "ensemble_container.hpp"
#include "extractor.hpp"

#include "utility.hpp"

template <class ChannelType>
ValueIterator<ChannelType>::ValueIterator( Extractor<ChannelType> *s_extractor, int s_id )
  : exemplar_states(),
    extractor( s_extractor ),
    id( s_id ),
    state_values(),
    incomplete_states(),
    unfinished_states()
{
  //  state_values.set_empty_key( Maybe<ChannelType>() );
}

template <class ChannelType>
ValueIterator<ChannelType>::ValueIterator( const ValueIterator &x )
  : exemplar_states( x.exemplar_states ),
    extractor( x.extractor ),
    id( x.id ),
    state_values( x.state_values ),
    incomplete_states( x.incomplete_states ),
    unfinished_states( x.unfinished_states )
{}

template <class ChannelType>
ValueIterator<ChannelType>::Exemplar::Exemplar( const ChannelType &e )
  : exemplar(), qm_cache( ChannelType( e ) )
{
  exemplar.advance_to( e.get_container()->time() );
  exemplar.add_mature( ChannelType( e ) );
  qm_cache.object->quantize_markovize();
}

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
    finished( false ),
    exemplar_state_index( -1 ),
    quantized_send_indices(),
    quantized_nosend_indices(),
    send_value( 0 ),
    nosend_value( 0 ),
    value( 0 )
{}

template <class ChannelType>
void ValueIterator<ChannelType>::add_state( const ChannelType &chan)
{
  /* Step 1: See if we already know the value. */

  Maybe< ChannelType > chanqm( chan );
  chanqm.object->quantize_markovize();

  typename value_map_t::const_iterator it = state_values.find( chanqm );
  if ( it != state_values.end() ) {
    return;
  }

  /* Step 2: If not, insert this quantized state (and its unquantized, unrationalized exemplar) into the lists. */

  VIValue new_vi_value;

  Exemplar exemplar( chan );

  exemplar_states.push_back( exemplar );
  new_vi_value.exemplar_state_index = exemplar_states.size() - 1;

  state_values[ chanqm ] = new_vi_value;
  incomplete_states.push_back( new_vi_value.exemplar_state_index );
  unfinished_states.push_back( new_vi_value.exemplar_state_index );
}

template <class ChannelType>
bool ValueIterator<ChannelType>::rationalize( void )
{
  if ( incomplete_states.empty() ) {
    return false;
  }

  /* Step 1: Look for vi_values that haven't been initialized */
  while ( !incomplete_states.empty() ) {
    size_t exemplar_index = incomplete_states.front();
    incomplete_states.pop_front();

    VIValue &vival( state_values[ exemplar_states[ exemplar_index ].qm_cache ] );
    double current_time = exemplar_states[ exemplar_index ].exemplar.time();

    assert( !vival.initialized );

    /* Step 2: Enumerate the nosend transitions */
    EnsembleContainer<ChannelType> nosend;

    nosend.advance_to( current_time );
    nosend.add_mature( ChannelType( exemplar_states[ exemplar_index ].exemplar.get_channel( 0 ).channel ) );
    nosend.advance_to( current_time + TIME_STEP );
    nosend.combine();

    for ( size_t i = 0; i < nosend.size(); i++ ) {
      double the_utility = UtilityMetric::utility( nosend.time(),
						   extractor->get_collector( nosend.get_channel( i ).channel ).get_packets(),
						   extractor->get_cross_traffic( nosend.get_channel( i ).channel ).get_packets() );
      extractor->reset( nosend.get_channel( i ).channel );
      ChannelType resultqm( nosend.get_channel( i ).channel );
      resultqm.quantize_markovize();
      WeightedChannel wc( nosend.get_channel( i ).probability, resultqm, the_utility );
      vival.quantized_nosend_indices.push_back( wc );
    }

    /* Step 3: Enumerate the quantized_send_indices */
    EnsembleContainer<ChannelType> send;
    send.advance_to( current_time );
    send.add_mature( ChannelType( exemplar_states[ exemplar_index ].exemplar.get_channel( 0 ).channel ) );
    extractor->get_pawn( send.get_channel( 0 ).channel ).send( Packet( 12000, id, 0, -1 ) );
    send.advance_to( current_time ); /* Send doesn't involve advancing! */
    send.combine();
  
    for ( size_t i = 0; i < send.size(); i++ ) {
      double the_utility = UtilityMetric::utility( nosend.time(),
						   extractor->get_collector( send.get_channel( i ).channel ).get_packets(),
						   extractor->get_cross_traffic( send.get_channel( i ).channel ).get_packets() );
      extractor->reset( send.get_channel( i ).channel );
      ChannelType resultqm( send.get_channel( i ).channel );
      resultqm.quantize_markovize();
      WeightedChannel wc( send.get_channel( i ).probability, resultqm, the_utility );
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

  return true;
}

template <class ChannelType>
void ValueIterator<ChannelType>::value_iterate( void )
{
  assert( incomplete_states.empty() );

  int changed;
  double max_diff;

  printf( "VALUE ITERATING on %lu states\n", unfinished_states.size() );

  do {
    changed = 0;
    max_diff = 0;

    for_each( unfinished_states.begin(), unfinished_states.end(),
	      [&state_values, &changed, &exemplar_states, &max_diff]( size_t ex_id )
	      {
		/* Lookup state */
		VIValue &vival( state_values[ exemplar_states[ ex_id ].qm_cache ] );

		/* Find value if we don't send */
		double nosend_value = 0;
		for( auto i = vival.quantized_nosend_indices.begin();
		     i != vival.quantized_nosend_indices.end();
		     i++ ) {
		  nosend_value += i->probability * ( i->reward + DISCOUNT * state_values[ i->channel ].value );
		}

		/* Find value if we do send */
		double send_value = 0;
		for( auto i = vival.quantized_send_indices.begin();
		     i != vival.quantized_send_indices.end();
		     i++ ) {
		  send_value += i->probability * ( i->reward + DISCOUNT * state_values[ i->channel ].value );
		}

		double new_value = send_value > nosend_value ? send_value : nosend_value;

		if ( (vival.nosend_value != nosend_value)
		     || (vival.send_value != send_value)
		     || (vival.value != new_value) ) {
		  changed++;
		}

		if ( nosend_value / vival.nosend_value - 1 > max_diff )
		  max_diff = nosend_value / vival.nosend_value - 1;
		if ( vival.nosend_value / nosend_value - 1 > max_diff )
		  max_diff = vival.nosend_value / nosend_value - 1;
		if ( send_value / vival.send_value - 1 > max_diff )
		  max_diff = send_value / vival.send_value - 1;
		if ( send_value / vival.send_value - 1 > max_diff )
		  max_diff = send_value / vival.send_value - 1;

		vival.nosend_value = nosend_value;
		vival.send_value = send_value;
		vival.value = new_value;
	      } );
    printf( "(%d / %f)\n", changed, max_diff );
  } while ( changed && (max_diff >= 0.01) );

  unfinished_states.clear();
}

template <class ChannelType>
bool ValueIterator<ChannelType>::should_i_send( const EnsembleContainer<ChannelType> &ensemble )
{
  double send_value = 0, nosend_value = 0;

  for ( size_t i = 0; i < ensemble.size(); i++ ) {
    if ( ensemble.get_channel( i ).erased ) {
      continue;
    }

    Maybe< ChannelType > chanqm( ensemble.get_channel( i ).channel );  
    chanqm.object->quantize_markovize();

    typename value_map_t::const_iterator it = state_values.find( chanqm );
    assert( it != state_values.end() );

    send_value += ensemble.get_channel( i ).probability * state_values[ chanqm ].send_value;
    nosend_value += ensemble.get_channel( i ).probability * state_values[ chanqm ].nosend_value;
  }

  //  printf( "SEND value = %f, NOSEND value = %f\n", send_value, nosend_value );

  return send_value > nosend_value;
}
