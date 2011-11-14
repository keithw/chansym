#ifndef CLOSE_HPP
#define CLOSE_HPP

#include <math.h>

inline bool close( double x, double y )
{
  return fabs( x - y ) < 1e-5;
}

static double rounder( double x )
{
  return nearbyint( x * 10000 ) / 10000;
}

#endif
