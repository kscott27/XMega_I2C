#include "Packet.h"

void Packet::resetContent()
{
  success_ = false;
  while( not_empty() )
  {
    get();
  }
}