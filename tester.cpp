#include "channel.hpp"

using namespace std;

class Pinger : public Actor
{
private:
  Channel *channel;
  double next_ping_time;
  double increment;
  string message;

public:
  void wakeup( void )
  {
    if ( time->now() >= next_ping_time ) {
      channel->send( message );
      next_ping_time += increment;
      time->sleep_until( Event( next_ping_time, this ) );
    }
  }
  
  Pinger( Time *time, Channel *s_channel, double s_inc, string s_msg )
    : Actor( time ),
      channel( s_channel ),
      next_ping_time( time->now() ),
      increment( s_inc ),
      message( s_msg )
  {
    wakeup();
  }

  Pinger( const Pinger & );
  Pinger & operator=( const Pinger & );
};

int main( void )
{
  Time tick;
  Channel chan( &tick, 900 );
  Pinger ping( &tick, &chan, 1, "ping\n" );
  Pinger pong( &tick, &chan, 0.9, "PONG\n" );

  while ( tick.tick() ) {}

  return 0;
}
