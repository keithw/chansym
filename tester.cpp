#include "simple_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"
#include "buffer.hpp"
#include "throughput.hpp"
#include "delay.hpp"

#include "series.cpp"
#include "simple_container.cpp"

int main( void )
{
  SimpleContainer< Series< Series<Pinger, Buffer>, Series<Throughput, Delay> > >
    overall( series( series( Pinger( 0.1 ), Buffer( 24000 ) ),
		     series( Throughput( 12000 ), Delay( 4 ) ) ) );

  while ( overall.tick() && (overall.time() < 1000) ) {}

  return 0;
}
