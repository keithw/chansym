#include "time.hpp"
#include "delay_channel.hpp"
#include "pinger.hpp"
#include "printer_channel.hpp"
#include "throughput_channel.hpp"
#include "buffer_channel.hpp"
#include "stats_channel.hpp"

using namespace std;

int main( void )
{
  Time tick;

  BufferChannel buf( &tick, 120000 );
  ThroughputChannel tp( &tick, 12000 );
  StatsChannel stats( &tick );

  buf.connect( &tp );
  tp.connect( &stats );

  Pinger ping( &tick, &buf, 0.1 );

  while ( tick.tick() && (tick.now() < 10000) ) {}

  printf( "t = %.2f. Average bps: %.2f. Average delay: %.2f seconds\n",
	  tick.now(), stats.average_bps(), stats.average_delay() );

  return 0;
}
