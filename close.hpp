#ifndef CLOSE_HPP
#define CLOSE_HPP

#include <math.h>

inline bool close( double x, double y )
{
  return fabs( x - y ) < 1e-8;
}

static double rounder( double x )
{
  return nearbyint( x * 1e8 ) * 1e-8;
}

#endif
