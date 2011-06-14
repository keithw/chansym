#include <stdio.h>
#include <assert.h>

#include "printer.hpp"
#include "container.hpp"

void Printer::send( Packet pack )
{
  printf( "At %.5f received packet id %d (sent %.5f)\n",
	  container->time(), pack.id, pack.send_time );
  container->receive( addr, pack );
}
