#ifndef COMMAGENT_H_
#define COMMAGENT_H_

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/interrupt.h>

#include "frt_queue.h"
#include "Command.h"
#include "Packet.h"

class CommAgent
{
public:
  CommAgent( uint8_t outPacketSize, uint8_t inPacketSize)
    : inPacket_(inPacketSize,NULL,10),
      outPacket_(outPacketSize,NULL,10)
  { }
  virtual bool transmit( Command & cmd ) = 0;
  virtual Packet & receive() = 0;

protected:
  virtual void writePacket( Command & cmd ) = 0;
  virtual void readPacket() = 0;
  virtual void resetPacket( Packet & packet ) = 0;

  Packet inPacket_;
  Packet outPacket_;
};

#endif // COMMAGENT_H_