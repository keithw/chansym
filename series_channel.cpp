#include <iostream>
#include <assert.h>

#include "series_channel.hpp"

SeriesChannel::SeriesChannel( Time *tick, Channel *s_a, Channel *s_b )
  : Channel( tick ), a( s_a ), b( s_b )
{
  a->connect( b );
}

SeriesChannel::~SeriesChannel()
{
  delete a;
  delete b;
}

SeriesChannel *SeriesChannel::get_first_series( void )
{
  SeriesChannel *ret = dynamic_cast<SeriesChannel *>( a );
  assert( ret );

  return ret;
}

SeriesChannel *SeriesChannel::get_second_series( void )
{
  SeriesChannel *ret = dynamic_cast<SeriesChannel *>( b );
  assert( ret );

  return ret;
}

SeriesChannel::SeriesChannel( const SeriesChannel &x )
  : Channel( x ), a( x.a->clone() ), b( x.b->clone() )
{}

SeriesChannel & SeriesChannel::operator=( const SeriesChannel &x )
{
  a->assign( x.a );
  b->assign( x.b );

  return *this;
}
