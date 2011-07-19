#include <sstream>

#include "embeddable_ensemble.hpp"

template <class ChannelType>
string EmbeddableEnsemble<ChannelType>::identify( void ) const
{
  ostringstream response;

  response << "<Ensemble (size=";
  response << (int)Parent::channels.size() - Parent::erased_count;
  response << ")\n";

  for ( unsigned int i = 0; i < Parent::channels.size(); i++ ) {
    response << i;
    response << "(p=";
    response.precision( 500 );
    response << Parent::channels[ i ].probability;
    response << ", utility=";
    response << Parent::channels[ i ].utility;
    response << "): " + Parent::channels[ i ].channel.identify();
    response << endl;
  }

  response << "</Ensemble (size=";
  response << (int)Parent::channels.size() - Parent::erased_count;
  response << ">\n";

  return response.str();
}
