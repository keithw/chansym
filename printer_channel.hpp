#ifndef PRINTER_CHANNEL_HPP
#define PRINTER_CHANNEL_HPP

#include "channel.hpp"

class PrinterChannel : public Channel {
public:
  PrinterChannel( Time *tick );

  void send( Packet pack );
  void wakeup( void ) {}

  CLONEMETHOD(PrinterChannel)
};

#endif
