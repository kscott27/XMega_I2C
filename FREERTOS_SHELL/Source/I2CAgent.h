#ifndef I2CAGENT_H_
#define I2CAGENT_H_

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/interrupt.h>

#include "CommAgent.h"
#include "I2CMaster.h"
#include "frt_queue.h"

class I2CAgent
  : public CommAgent
{
public:
  inline I2CAgent( uint8_t packetSize = 10 ) 
    : CommAgent(packetSize)
  {}
  virtual bool transmit( Command & cmd );
  // virtual Packet receive();
  void setI2CDriver( I2CMaster * d ) { driver_ = d; }
  void setSlaveAddr( uint8_t addr );

protected:

  virtual void writePacket( Command & cmd );
  virtual void readPacket();
  virtual void resetPacket( Packet & packet );

  I2CMaster * driver_;
  uint8_t slaveAddr_;
  uint8_t writeCommand_;
  uint8_t readCommand_;

};

#endif // I2CAGENT_H_