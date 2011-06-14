#include <stdio.h>

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
  SimpleContainer< Series< Series<Pinger, Buffer>, Series< Series<Throughput, Delay>, Printer > > >
    overall( series( series( Pinger( 0.1 ), Buffer( 24000 ) ),
		     series( series( Throughput( 12000 ), Delay( 4 ) ),
			     Printer() ) ) );

  while ( overall.tick() && (overall.time() < 500) ) {}

  SimpleContainer< Series< Series<Pinger, Buffer>, Series< Series<Throughput, Delay>, Printer > > >
    o2( series( series( Pinger( 0.5 ), Buffer( 1900 ) ),
		series( series( Throughput( 12900 ), Delay( 2 ) ),
			Printer() ) ) );

  o2 = overall;

  overall.tick();
  o2.tick();
  overall.tick();
  overall.tick();
  o2.tick();
  o2.tick();

  assert( overall == o2 );

  while ( o2.tick() && (o2.time() < 1000) ) {}

  return 0;
}
