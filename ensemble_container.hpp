#ifndef ENSEMBLE_CONTAINER_HPP
#define ENSEMBLE_CONTAINER_HPP

#include <queue>
#include "container.hpp"
#include "close.hpp"

using namespace std;

template <class ChannelType>
class EnsembleContainer : public Container
{
private:
  class WeightedChannel {
  public:
    double probability;
    ChannelType channel;
    bool erased;

    double delay;
    double utility;

    WeightedChannel( double s_p, ChannelType s_c )
      : probability( s_p ), channel( s_c ), erased( false ), delay( -5 ), utility( -5 )
    {}

    bool operator==( const WeightedChannel &x ) const
    {
      /*
      printf( "Comparing two WeightedChannels: prob=%d, chan=%d, erased=%d\n",
	      (probability == x.probability),
	      (channel == x.channel),
	      (erased == x.erased) );
      */
      return ( (float)probability == (float)x.probability ) && (channel == x.channel) && (erased == x.erased);
    }

    friend size_t hash_value( const WeightedChannel &x )
    {
      size_t seed = 0;
      boost::hash_combine( seed, (float)x.probability );
      boost::hash_combine( seed, x.channel );
      boost::hash_combine( seed, x.erased );
      return seed;
    }
  };

  class PendingFork {
  public:
    int orig_addr;
    double my_probability;
    typename ChannelType::ForkState fs;

    PendingFork( int s_orig_addr, double s_my_prob, typename ChannelType::ForkState s_fs )
      : orig_addr( s_orig_addr ), my_probability( s_my_prob ), fs( s_fs )
    {}

    bool operator==( const PendingFork &x ) const
    {
      return (orig_addr == x.orig_addr) && (my_probability == x.my_probability) && (fs == x.fs);
    }
  };

protected:
  double the_time;
  vector<WeightedChannel> channels;
  queue<PendingFork> fork_queue;
  int erased_count;

private:
  bool printing;
  bool forking;

  void compact( void );
  void assert_normalized( void );
  
public:
  EnsembleContainer();
  EnsembleContainer( ChannelType s_channel );
  EnsembleContainer( const EnsembleContainer<ChannelType> &x );
  EnsembleContainer( double s_time );

  bool tick( void );
  void advance_to( double advance_time );

  void combine( void );
  bool converged( void );

  virtual void make_wakeup( double time, int source_addr, int sort_order )
  {
    wakeups.push( Event( time, source_addr, sort_order ) );
  }

  virtual void clear_wakeups( void ) { wakeups.clear(); }
  void clear_wakeups( int source_addr );

  void sleep_until( double time, int source_addr, int sort_order=0 ) { make_wakeup( time, source_addr, sort_order ); }
  void signal_sendable( int ) {}
  bool can_send( int ) { return( true ); }
  void receive( int source_addr, Packet pack );
  virtual double time( void ) const { return the_time; }
  void fork( int source_addr, double my_probability, Channel::ForkState *fs );
  double probability( int source_addr );

  void add( ChannelType s_channel );
  void add_mature( ChannelType s_channel );
  void set_printing( bool s_printing ) { printing = s_printing; }
  void set_follow_all_forks( bool s_forking ) { forking = s_forking; }

  bool operator==( const EnsembleContainer<ChannelType> &x ) const;

  unsigned int size( void ) const { return channels.size(); }
  WeightedChannel & get_channel( int address );

  bool live( void ) { return !wakeups.empty(); }
  double next_time( void );
  void erase( int address );
  void normalize( void );
  void prune( double threshold );
  void execute_fork( void );

  string identify( void ) const;

  int get_erased_count( void ) { return erased_count; }

  void print_wakeups( void );

  class equal_channels {
  public:
    bool operator() ( const ChannelType *a, const ChannelType *b ) const
    {
      if ( (a == NULL) && (b == NULL) ) {
	return true;
      }

      if ( (a == NULL) || (b == NULL) ) {
	return false;
      }

      return ((*a) == (*b));
    }

    bool operator() ( const WeightedChannel *a, const WeightedChannel *b ) const
    {
      if ( (a == NULL) && (b == NULL) ) {
	return true;
      }

      if ( (a == NULL) || (b == NULL) ) {
	return false;
      }

      return ((*a) == (*b));
    }
  };

  class channel_hash {
  public:
    size_t operator() ( const ChannelType *a ) const
    {
      return hash_value( *a );
    }

    size_t operator() ( const WeightedChannel *a ) const
    {
      return hash_value( *a );
    }
  };

  size_t hash( void ) const;
};

template <class ChannelType>
size_t hash_value( EnsembleContainer<ChannelType> const &x )
{
  return x.hash();
}

#endif
