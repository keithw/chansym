#include "time.hpp"
#include "delay_channel.hpp"
#include "pinger.hpp"

using namespace std;

int main( void )
{
  Time tick;
  DelayChannel chan( &tick, 900 );
  Pinger ping( &tick, &chan, 1, "ping\n" );
  Pinger pong( &tick, &chan, 0.9, "PONG\n" );

  while ( tick.tick() ) {}

  return 0;
}
