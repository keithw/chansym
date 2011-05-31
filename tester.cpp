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
  Time tick2;

  SeriesChannel chan( &tick,
		      new Pinger( &tick, 0.01 ),
		      new SeriesChannel( &tick,
					 new BufferChannel( &tick, 120000 ),
					 new SeriesChannel( &tick,
							    new ThroughputChannel( &tick, 5000 ),
							    new SeriesChannel( &tick,
									       new StatsChannel( &tick ),
									       new PrinterChannel( &tick ) ) ) ) );

  SeriesChannel chan2( &tick2,
		      new Pinger( &tick2, 0.9997 ),
		      new SeriesChannel( &tick,
					 new BufferChannel( &tick2, 120000 ),
					 new SeriesChannel( &tick2,
							    new ThroughputChannel( &tick2, 12000 ),
							    new SeriesChannel( &tick2,
									       new StatsChannel( &tick2 ),
									       new PrinterChannel( &tick2 ) ) ) ) );

  StatsChannel *stats = dynamic_cast<StatsChannel *>( chan.get_second_series()->get_second_series()->get_second_series()->get_first() );
  assert( stats );

  BufferChannel *buf = dynamic_cast<BufferChannel *>( chan.get_second_series()->get_first() );
  assert( buf );

  buf->send( Packet( 10000, 1, 50, tick.now() ) );

  chan = chan2;

  while ( tick.tick() && (tick.now() < 10000) ) {}

  printf( "t = %.2f. Average bps: %.2f. Average delay: %.2f seconds\n",
	  tick.now(), stats->average_bps(), stats->average_delay() );

  return 0;
}
