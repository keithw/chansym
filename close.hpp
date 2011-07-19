#ifndef CLOSE_HPP
#define CLOSE_HPP

#include <math.h>

inline bool close( double x, double y )
{
  return fabs( x - y ) < 1e-10;
}

#endif
