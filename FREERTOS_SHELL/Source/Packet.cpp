#include "Packet.h"

void Packet::resetContent()
{
  while( not_empty() )
  {
    get();
  }
}