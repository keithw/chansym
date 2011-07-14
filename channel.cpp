#include "channel.hpp"
#include "container.hpp"

Channel * Channel::get_container_channel( void )
{
  Channel *ch = dynamic_cast<Channel *>( container );
  assert( ch );
  return ch;
}
