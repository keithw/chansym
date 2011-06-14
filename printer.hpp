#ifndef PRINTER_HPP
#define PRINTER_HPP

#include "channel.hpp"

class Printer : public Channel {
public:
  Printer() {}

  void send( Packet pack );
  bool sendable( void ) { return true; }

  bool operator==( const Printer & ) const { return true; }
};

#endif
