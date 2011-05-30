#ifndef ACTOR_HPP
#define ACTOR_HPP

class Time;

class Actor {
private:
  bool is_clone;

protected:
  Time *time;

  void set_clone( void ) { is_clone = true; }

public:
  Actor( Time *s_time ) : is_clone( false ), time( s_time ) {}

  virtual void wakeup( void ) = 0;
  virtual ~Actor() {}

  bool get_is_clone( void ) { return is_clone; }

  Actor( const Actor & ) : is_clone( true ), time( NULL ) {}
  Actor & operator=( const Actor & ) { return *this; }
};

#endif
