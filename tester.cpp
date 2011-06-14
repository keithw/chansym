#include "simple_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"
#include "buffer.hpp"
#include "throughput.hpp"
#include "delay.hpp"

int main( void )
{
  SimpleContainer< Series< Series<Pinger, Buffer>, Series<Throughput, Delay> > >
    overall( Series< Series<Pinger, Buffer>, Series<Throughput, Delay> >
	     ( Series<Pinger, Buffer>( Pinger( 0.1 ), Buffer( 24000 ) ),
	       Series<Throughput, Delay>( Throughput( 12000 ), Delay( 4 ) ) ) );

  while ( overall.tick() && (overall.time() < 1000) ) {}

  return 0;
}
