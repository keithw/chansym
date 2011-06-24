#include <iostream>
#include <assert.h>
#include <sstream>

#include "buffer.hpp"
#include "container.hpp"

Buffer::Buffer( int s_size, int initial_num, int initial_size )
  : Channel(), size( s_size ), occupancy( 0 ), contents(),
    tmp_initial_num( initial_num ), tmp_initial_size( initial_size )
{}

void Buffer::init( void )
{
  for ( int i = 0; i < tmp_initial_num; i++ ) {
    send( Packet( tmp_initial_size, -1, -1, container->time() ) );
  }
}

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
