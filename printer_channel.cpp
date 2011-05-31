#include <stdio.h>
#include <assert.h>

#include "printer_channel.hpp"

PrinterChannel::PrinterChannel( Time *tick )
  : Channel( tick ) 
{}

void PrinterChannel::send( Packet pack )
{
  printf( "At %.5f received packet id %d (sent %.5f)\n",
	  time->now(), pack.id, pack.send_time );
  if ( dest ) { dest->send( pack ); }
}
