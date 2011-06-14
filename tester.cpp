#include "simple_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"
#include "buffer.hpp"
#include "throughput.hpp"

int main( void )
{
  Series<Pinger, Buffer> orig( Pinger( 0.1 ), Buffer( 24000 ) );
  Series<Throughput, Printer> fini( Throughput( 12000 ), Printer() );
  
  Series< Series<Pinger, Buffer>, Series<Throughput, Printer> > both( orig, fini );

  SimpleContainer< Series< Series<Pinger, Buffer>, Series<Throughput, Printer> > > overall( both );

  while ( overall.tick() && (overall.time() < 1000) ) {}

  return 0;
}
