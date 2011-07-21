#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "packet.hpp"

class UtilityMetric {
private:

  static double utility_single( double base_time, vector<ScheduledPacket> x, bool penalize_delay )
  {
    double util = 0;
    
    for ( vector<ScheduledPacket>::const_iterator i = x.begin();
	  i != x.end();
	  i++ ) {
      double future = i->delivery_time - base_time;
      assert( future >= 0 );
      assert( future < 200 );

      double this_packet = i->packet.length / exp( future );

      if ( penalize_delay ) {
	double delay = i->delivery_time - i->packet.send_time;
	assert( delay >= 0 );
	assert( delay < 12 );
	double penalty = this_packet * (delay / 12.0);
	assert( isfinite( penalty ) );
	this_packet -= penalty;
      }

      assert( isfinite( this_packet ) );

      util += this_packet;

      assert( isfinite( util ) );
    }

    assert( isfinite( util ) );
    
    return util;
  }

public:
  static double utility( double base_time,
			 vector<ScheduledPacket> &real_traffic,
			 vector<ScheduledPacket> &cross_traffic )
  {
    return 1 * utility_single( base_time, real_traffic, false )
      + 2 * utility_single( base_time, cross_traffic, false );
  }
};

#endif
