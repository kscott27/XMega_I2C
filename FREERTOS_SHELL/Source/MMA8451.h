/*
 * MAX31855.h
 *
 * Created: 2/27/2018 11:57:36 PM
 *  Author: Kevin
 */ 


#ifndef MMA8451_H_
#define MMA8451_H_

#define SLAVE_ADDR 29

#include "I2CMaster.h"
#include "I2CAgent.h"
#include "Command.h"
#include "Packet.h"
#include "emstream.h"

class MMA8451
{
public:

  class ActiveCommand
    : public Command
  {
  public:
    inline ActiveCommand()
      : reg_(MMA8451::CTRL_REG1)
    { }
    void writePacket( Packet & packet );
  protected:
    uint8_t reg_;
    const uint8_t data_ = 0x01;
  };

  class QueryXRegCommand
    : public Command
  {
  public:
    inline QueryXRegCommand()
      : reg_(MMA8451::OUT_X_MSB)
    { }
    void writePacket( Packet & packet );
  protected:
    uint8_t reg_;
  };

  class QueryYRegCommand
    : public Command
  {
  public:
    inline QueryYRegCommand()
      : reg_(MMA8451::OUT_Y_MSB)
    { }
    void writePacket( Packet & packet );
  protected:
    uint8_t reg_;
  };

  class QueryZRegCommand
    : public Command
  {
  public:
    inline QueryZRegCommand()
      : reg_(MMA8451::OUT_Z_MSB)
    { }
    void writePacket( Packet & packet );
  protected:
    uint8_t reg_;
  };
  
  MMA8451( I2CMaster * d, emstream * s );
  
  bool is_ready(void);

  uint16_t getXReading();
  uint16_t getYReading();
  uint16_t getZReading();

  static const uint8_t CTRL_REG1 = 0x2A;
  static const uint8_t OUT_X_MSB = 0x01;
  static const uint8_t OUT_Y_MSB = 0x03;
  static const uint8_t OUT_Z_MSB = 0x05;

protected:

  I2CMaster * driver_;
  emstream * p_serial;
  I2CAgent * i2cAgent_;
  uint8_t slaveAddr_;
  uint16_t reading_;
  ActiveCommand * activeCommand_;
  QueryXRegCommand * queryX_;
  QueryYRegCommand * queryY_;
  QueryZRegCommand * queryZ_;
  static const uint8_t outPacketSize_ = 3;
  static const uint8_t inPacketSize_ = 2;
  
};


#endif /* MMA8451_H_ */