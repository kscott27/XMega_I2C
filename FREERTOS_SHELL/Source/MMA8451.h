/*
 * MAX31855.h
 *
 * Created: 2/27/2018 11:57:36 PM
 *  Author: Kevin
 */ 


#ifndef MMA8451_H_
#define MMA8451_H_

#define SLAVE_ADDR 29
#define ADDR_CHANGE_0 170
#define ADDR_CHANGE_1 165

#include "I2CMaster.h"
#include "I2CAgent.h"
#include "Command.h"
#include "frt_queue.h"
#include "emstream.h"

class MMA8451
{
public:

  class ActiveCommand
    : public Command
  {
  public:
    typedef frt_queue< uint8_t > Packet;
    inline ActiveCommand()
      : reg_(MMA8451::CTRL_REG1)
    { }
    void writePacket( Packet & packet );
  protected:
    uint8_t reg_;
    const uint8_t data_ = 0x01;
  };
  
  MMA8451(I2CMaster * d);
  MMA8451( I2CMaster * d, emstream * s );
  
  bool is_ready(void);
  
  bool takeReading(void);
  
  uint16_t getReading(void);

  static const uint8_t CTRL_REG1 = 0x2A;

protected:

  I2CMaster * driver_;
  emstream * p_serial;
  I2CAgent * i2cAgent_;
  uint8_t slaveAddr_;
  uint16_t reading_;
  ActiveCommand * activeCommand_;
  static const uint8_t outPacketSize_ = 3;
  static const uint8_t inPacketSize_ = 2;
  
};


#endif /* MMA8451_H_ */