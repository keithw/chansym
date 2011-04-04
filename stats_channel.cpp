#include <iostream>
#include <assert.h>

#include "stats_channel.hpp"

StatsChannel::StatsChannel( Time *tick )
  : Channel( tick ), total_bits( 0 ), total_delay( 0 )
{}

void StatsChannel::send( Packet pack )
{
  total_bits += pack.length;
  total_delay += time->now() - pack.send_time;

  if ( dest ) dest->send( pack );
}

void StatsChannel::wakeup( void )
{
  if ( src ) src->wakeup();
}

double StatsChannel::average_bps( void )
{
  return total_bits / time->now();
}

double StatsChannel::average_delay( void )
{
  return total_delay / time->now();
}
