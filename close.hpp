#ifndef CLOSE_HPP
#define CLOSE_HPP

#include <math.h>

const double TIME_STEP = 0.01;
const double PROB_STEP = 0.1;
const int PACKET_SIZE = 12000;

inline bool close( double x, double y )
{
  return fabs( x - y ) < 1e-12;
}

inline static double rounder( double x )
{
  return nearbyint( x * 1e12 ) * 1e-12;
}

inline static double variable_round( double x, double increment )
{
  return nearbyint( x / increment ) * increment;
}

inline static double quantize_time( double t )
{
  return variable_round( t, TIME_STEP );
}

inline static double quantize_probability( double p )
{
  return variable_round( p, PROB_STEP );
}

inline static int quantize_length( int x )
{
  return (x / PACKET_SIZE) * PACKET_SIZE;
}

#endif
