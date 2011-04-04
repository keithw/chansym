#include "time.hpp"
#include "delay_channel.hpp"
#include "pinger.hpp"
#include "printer_channel.hpp"
#include "throughput_channel.hpp"
#include "buffer_channel.hpp"
#include "stats_channel.hpp"
#include "series_channel.hpp"

using namespace std;

int main( void )
{
  Time tick;

  SeriesChannel chan( &tick,
		      new BufferChannel( &tick, 120000 ),
		      new ThroughputChannel( &tick, 12000 ) );

  StatsChannel stats( &tick );

  chan.connect( &stats );

  Pinger ping( &tick, &chan, 0.9997 );

  while ( tick.tick() && (tick.now() < 10000) ) {}

  printf( "t = %.2f. Average bps: %.2f. Average delay: %.2f seconds\n",
	  tick.now(), stats.average_bps(), stats.average_delay() );

  return 0;
}
