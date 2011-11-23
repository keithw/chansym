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
      double future = (i->delivery_time - base_time);
      assert( future >= 0 );
      assert( future < 200 );

      double this_packet = i->packet.length / exp2( future * 1e-6 );

      if ( penalize_delay ) {
	assert( i->packet.send_time >= 0 );
	double delay = i->delivery_time - i->packet.send_time;
	assert( delay >= 0 );
	assert( delay < 30 );
	double penalty = this_packet * (1 - exp2( - delay / 100.0 ));
	assert( penalty < this_packet );
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
    double util = utility_single( base_time, real_traffic, false )
      + 1.01 * utility_single( base_time, cross_traffic, false );

    assert( isfinite( util ) );
    return util;
  }
};

#endif
