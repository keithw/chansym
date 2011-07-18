#include <sstream>

#include "ensemble_container.hpp"
#include "utility_ensemble.hpp"

template <class ChannelType>
string UtilityEnsemble<ChannelType>::identify( void ) const
{
  ostringstream response;

  response << "<Ensemble (size=";
  response << (int)Parent::channels.size() - Parent::erased_count;
  response << ")\n";

  for ( unsigned int i = 0; i < Parent::channels.size(); i++ ) {
    response << i;
    response << " (delay=";
    response << Parent::channels[ i ].delay;
    response << "): " + Parent::channels[ i ].channel.identify();
    response << endl;
  }

  response << "</Ensemble (size=";
  response << (int)Parent::channels.size() - Parent::erased_count;
  response << ">\n";

  return response.str();
}
