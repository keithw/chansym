#include <iostream>

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
