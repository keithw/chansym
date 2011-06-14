#include "simple_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"

int main( void )
{
  SimpleContainer< Series<Pinger, Printer> > channel( Series<Pinger, Printer>( Pinger( 1 ), Printer() ) );

  while ( channel.tick() && (channel.time() < 1000) ) {}

  SimpleContainer< Series<Pinger, Printer> > channel2( channel );
  
  while ( channel2.tick() && (channel2.time() < 2000) ) {}

  return 0;
}
