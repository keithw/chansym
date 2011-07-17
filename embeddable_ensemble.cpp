#include <sstream>

#include "embeddable_ensemble.hpp"

template <class ChannelType>
string EmbeddableEnsemble<ChannelType>::identify( void )
{
  ostringstream response;

  response << "<Ensemble (size=";
  response << (int)Parent::channels.size() - Parent::erased_count;
  response << ")\n";

  for ( unsigned int i = 0; i < Parent::channels.size(); i++ ) {
    response << i;
    response << " (utility=";
    response << Parent::channels[ i ].utility;
    response << "): " + Parent::channels[ i ].channel.identify();
    response << endl;
  }

  response << "</Ensemble (size=";
  response << (int)Parent::channels.size() - Parent::erased_count;
  response << ">\n";

  return response.str();
}
