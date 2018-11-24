#ifndef COMMAND_H_
#define COMMAND_H_

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/interrupt.h>

#include "Packet.h"

class Command
{
public:
  inline Command() { }
  virtual void writePacket( Packet & packet ) = 0;
};


#endif // COMMAND_H_