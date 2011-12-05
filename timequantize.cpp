#include <boost/functional/hash.hpp>
#include <sstream>
#include <math.h>
#include <algorithm>

#include "timequantize.hpp"
#include "container.hpp"
#include "close.hpp"

TimeQuantize::TimeQuantize( double s_interval )
  : Channel(), interval( s_interval ), contents()
{}

void TimeQuantize::send( Packet pack )
{
  double epoch = floor(container->time() / interval);

  double delivery_time = (epoch + 1) * interval;

  assert( delivery_time >= container->time() - 0.0001 );
  assert( delivery_time <= container->time() + interval + 0.0001 );

  contents.push_back( ScheduledPacket( delivery_time, pack ) );
  container->sleep_until( delivery_time, addr );
}

void TimeQuantize::wakeup( void )
{
  ScheduledPacket pack = contents.front();
  contents.pop_front();
  assert( container->time() == pack.delivery_time );
  container->receive( addr, pack.packet );
}

string TimeQuantize::identify( void ) const
{
  ostringstream response;
  response << "TimeQuantize( ";
  response << interval;

  for ( std::list<ScheduledPacket>::const_iterator i = contents.begin();
	i != contents.end();
	i++ ) {
    response << ", < ";
    response << i->delivery_time;
    response << ", ";
    response << i->packet.src;
    response << ", ";
    response << i->packet.id;
    response << ", ";
    response << i->packet.send_time;

    response << " >";
  }

  response << " )";

  return response.str();
}

size_t hash_value( TimeQuantize const & ch )
{
  size_t seed = 0;
  boost::hash_combine( seed, ch.interval );
  boost::hash_combine( seed, ch.contents );
  return seed;
}

void TimeQuantize::quantize_markovize( void )
{
  interval = quantize_time( interval );
  double now = container->time();

  for_each( contents.begin(), contents.end(),
	    [now]( ScheduledPacket &x ) { x.delivery_time = quantize_time( x.delivery_time - now );
	      x.packet.quantize_markovize( now ); } );
}
