#include "time.hpp"
#include "delay_channel.hpp"
#include "pinger.hpp"
#include "printer_channel.hpp"
#include "throughput_channel.hpp"
#include "buffer_channel.hpp"

using namespace std;

int main( void )
{
  Time tick;

  BufferChannel buf( &tick, 120000 );
  ThroughputChannel tp( &tick, 12000 );
  PrinterChannel printer( &tick );

  buf.connect( &tp );
  tp.connect( &printer );

  Pinger ping( &tick, &buf, 0.01 );

  while ( tick.tick() ) {}

  return 0;
}
