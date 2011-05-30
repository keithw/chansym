#ifndef ACTOR_HPP
#define ACTOR_HPP

class Time;

class Actor {
protected:
  Time *time;

public:
  Actor( Time *s_time ) : time( s_time ) {}

  virtual void wakeup( void ) = 0;
  virtual ~Actor() {}
};

#endif
