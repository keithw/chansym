#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "packet.hpp"

const double DISCOUNT = 0.99;

class UtilityMetric {
private:

  static double utility_single( double, vector<ScheduledPacket> x, bool penalize_delay )
  {
    double util = 0;
    
    for ( vector<ScheduledPacket>::const_iterator i = x.begin();
	  i != x.end();
	  i++ ) {
      double this_packet = i->packet.length;

      if ( penalize_delay ) {
	assert( false );
      }

      util += this_packet;
    }

    return util;
  }

public:
  static double utility( double base_time,
			 vector<ScheduledPacket> &real_traffic,
			 vector<ScheduledPacket> &cross_traffic )
  {
    double util = utility_single( base_time, real_traffic, false )
      + 1.01 * utility_single( base_time, cross_traffic, false );

    return util;
  }
};

#endif
