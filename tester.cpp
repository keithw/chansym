#include "simple_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"
#include "buffer.hpp"
#include "throughput.hpp"
#include "delay.hpp"

int main( void )
{
  Series<Pinger, Buffer> orig( Pinger( 0.1 ), Buffer( 24000 ) );
  Series<Delay, Printer> fini( Delay( 4 ), Printer() );
  
  Series< Series<Pinger, Buffer>, Series<Delay, Printer> > both( orig, fini );

  SimpleContainer< Series< Series<Pinger, Buffer>, Series<Delay, Printer> > > overall( both );

  while ( overall.tick() && (overall.time() < 1000) ) {}

  return 0;
}
