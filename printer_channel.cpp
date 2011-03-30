#include <iostream>
#include <assert.h>

#include "printer_channel.hpp"

PrinterChannel::PrinterChannel( Time *tick )
  : Channel( tick ) 
{}

void PrinterChannel::send( Packet pack )
{
  cout << time->now() << "\t" << pack.src << " " << pack.length << endl;
}
