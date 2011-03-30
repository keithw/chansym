#include "time.hpp"
#include "delay_channel.hpp"
#include "pinger.hpp"
#include "printer_channel.hpp"
#include "throughput_channel.hpp"

using namespace std;

int main( void )
{
  Time tick;

  ThroughputChannel chan( &tick, 12000 );
  PrinterChannel printer( &tick );
  chan.connect( &printer );

  Pinger ping( &tick, &chan, 0.00001 );

  while ( tick.tick() ) {}

  return 0;
}
