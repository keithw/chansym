#include <iostream>
#include <assert.h>
#include <sstream>
#include <boost/functional/hash.hpp>

#include "buffer.hpp"
#include "container.hpp"

Buffer::Buffer( int s_size, int initial_num, int initial_size )
  : Channel(), size( s_size ), occupancy( 0 ), contents(),
    tmp_initial_num( initial_num ), tmp_initial_size( initial_size )
{}

void Buffer::init( void )
{
  for ( int i = 0; i < tmp_initial_num; i++ ) {
    send( Packet( tmp_initial_size, -1, -1, -1 ) );
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

string Buffer::identify( void ) const
{
  ostringstream response;
  response << "Buffer( ";
  response << size;
  response << ", ";
  response << occupancy;

  /*
  for ( std::list<Packet>::const_iterator i = contents.begin();
	i != contents.end();
	i++ ) {
    response << ", < ";
    response << i->src;
    response << ", ";
    response << i->id;
    response << ", ";
    response << i->send_time;
    response << " >";
  }
  */

  response << " )";

  return response.str();
}

size_t hash_value( Buffer const & ch )
{
  size_t seed = 0;

  boost::hash_combine( seed, ch.size );
  boost::hash_combine( seed, ch.occupancy );
  boost::hash_combine( seed, ch.contents );

  return seed;
}
