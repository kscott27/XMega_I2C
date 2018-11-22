#ifndef COMMAGENT_H_
#define COMMAGENT_H_

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/interrupt.h>

#include "frt_queue.h"
#include "Command.h"

class CommAgent
{
public:
  CommAgent( uint8_t packetSize = 10 )
    : inPacket_(packetSize,NULL,10),
      outPacket_(packetSize,NULL,10)
  { }
  virtual bool transmit( Command & cmd ) = 0;
  virtual bool receive() = 0;

protected:
  typedef frt_queue< uint8_t > Packet;

  virtual void writePacket() = 0;
  virtual void readPacket() = 0;
  virtual void resetPacket( Packet & packet ) = 0;

  Packet inPacket_;
  Packet outPacket_;
};

#endif // COMMAGENT_H_