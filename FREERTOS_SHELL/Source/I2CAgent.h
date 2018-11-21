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

  typedef frt_queue< uint8_t > Packet;

  inline I2CAgent() {}
  virtual bool transmit( Command & cmd );
  virtual bool receive();
  void setI2CDriver( I2CMaster * d ) { driver_ = d; }
  void setSlaveAddr( uint8_t addr );

protected:

  virtual void writePacket();
  virtual void readPacket();
  virtual void resetPacket( Packet & packet );

  I2CMaster * driver_;
  uint8_t slaveAddr_;
  uint8_t writeCommand_;
  uint8_t readCommand_;

};

#endif // I2CAGENT_H_