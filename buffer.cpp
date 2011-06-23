#include <iostream>
#include <assert.h>
#include <sstream>

#include "buffer.hpp"
#include "container.hpp"

Buffer::Buffer( int s_size )
  : Channel(), size( s_size ), occupancy( 0 ), contents()
{}

void Buffer::uncork( void )
{
  while ( (!contents.empty()) && container->can_send( addr ) ) {
    Packet pack = contents.front();
    contents.pop_front();
    occupancy -= pack.length;
    assert( occupancy >= 0 );
    container->receive( addr, pack );
  }
}

void Buffer::send( Packet pack )
{
  if ( occupancy + pack.length <= size ) { /* tail drop behavior */
    contents.push_back( pack );
    occupancy += pack.length;
  }

  uncork();
}

string Buffer::identify( void )
{
  ostringstream response;
  response << "Buffer( ";
  response << size;
  response << ", ";
  response << occupancy;
  response << " )";

  return response.str();
}
