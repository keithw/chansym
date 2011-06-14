#include "simple_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"

int main( void )
{
  SimpleContainer< Series<Pinger, Printer> > channel( Series<Pinger, Printer>( Pinger( 1 ), Printer() ) );

  while ( channel.tick() && (channel.time() < 1000) ) {}

  return 0;
}
