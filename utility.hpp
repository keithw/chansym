#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "packet.hpp"

class UtilityMetric {
private:

  static double utility_single( vector<ScheduledPacket> x, bool penalize_delay )
  {
    double util = 0;
    
    for ( vector<ScheduledPacket>::const_iterator i = x.begin();
	  i != x.end();
	  i++ ) {
      util += 20;
      
      if ( penalize_delay ) {
	double delay = i->delivery_time - i->packet.send_time;
	assert( delay >= 0 );
	assert( delay < 20 );
	util -= delay;
      }
    }
    
    return util;
  }

public:
  static double utility( vector<ScheduledPacket> &real_traffic,
			 vector<ScheduledPacket> &cross_traffic )
  {
    return utility_single( real_traffic, false ) + 0.001 * utility_single( cross_traffic, true );
  }
};

#endif
