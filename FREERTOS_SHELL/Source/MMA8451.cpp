/*
 * MAX31855.cpp
 *
 * Created: 2/27/2018 11:58:06 PM
 *  Author: Kevin
 */ 

#include "MMA8451.h"

//-------------------------------------------------------------------------------------
/** This constructor creates an object to use when interacting with the SPI registers on the MAX31855
    Thermocouple Analog to Digital Converter chip.
 *  @param SPIM A pointer to an SPI_master object
 */
MMA8451::MMA8451(I2CMaster * d)
  : driver_(d),
    i2cAgent_(new I2CAgent(outPacketSize_, inPacketSize_)),
    slaveAddr_(SLAVE_ADDR),
    activeCommand_(new ActiveCommand())
{
  i2cAgent_->setI2CDriver(d);
  i2cAgent_->setSlaveAddr(slaveAddr_);
  i2cAgent_->transmit(*activeCommand_);
}

MMA8451::MMA8451(I2CMaster * d, emstream * s)
  : driver_(d),
    p_serial(s),
    i2cAgent_(new I2CAgent(s, outPacketSize_, inPacketSize_)),
    slaveAddr_(SLAVE_ADDR),
    accelData_(new Data()),
    activeCommand_(new ActiveCommand()),
    queryX_(new QueryXRegCommand()),
    queryY_(new QueryYRegCommand()),
    queryZ_(new QueryZRegCommand())
{
  i2cAgent_->setI2CDriver(d);
  i2cAgent_->setSlaveAddr(slaveAddr_);
}

void MMA8451::ActiveCommand::writePacket( Packet & packet )
{
  packet.put(reg_);
  packet.put(data_);
}

void MMA8451::QueryXRegCommand::writePacket( Packet & packet )
{
  packet.put(reg_);
}

void MMA8451::QueryYRegCommand::writePacket( Packet & packet )
{
  packet.put(reg_);
}

void MMA8451::QueryZRegCommand::writePacket( Packet & packet )
{
  packet.put(reg_);
}

bool MMA8451::is_ready()
{
  return driver_->is_ready(slaveAddr_);
}

bool MMA8451::takeReading()
{
  return i2cAgent_->transmit(*queryX_);
}

uint16_t MMA8451::getXReading()
{
  i2cAgent_->transmit(*activeCommand_);
  i2cAgent_->transmit(*queryX_);
  Packet & xData = i2cAgent_->receive();
  uint16_t data = 0;
  if(xData.validData())
    data = ((uint16_t) xData.get() << 8) | ((uint16_t) xData.get());
  return data;
}

uint16_t MMA8451::getYReading()
{
  i2cAgent_->transmit(*activeCommand_);
  i2cAgent_->transmit(*queryY_);
  Packet & yData = i2cAgent_->receive();
  uint16_t data = 0;
  if(yData.validData())
    data = ((uint16_t) yData.get() << 8) | ((uint16_t) yData.get());
  return data;
}

uint16_t MMA8451::getZReading()
{
  i2cAgent_->transmit(*activeCommand_);
  i2cAgent_->transmit(*queryZ_);
  Packet & zData = i2cAgent_->receive();
  uint16_t data = 0;
  if(zData.validData())
    data = ((uint16_t) zData.get() << 8) | ((uint16_t) zData.get());
  return data;
}

void MMA8451::getReading( uint16_t * accelData )
{
  i2cAgent_->transmit(*activeCommand_);
  i2cAgent_->transmit(*queryX_);
  Packet & xData = i2cAgent_->receive();
  i2cAgent_->transmit(*queryY_);
  Packet & yData = i2cAgent_->receive();
  i2cAgent_->transmit(*queryZ_);
  Packet & zData = i2cAgent_->receive();
  accelData[0] = ((uint16_t) xData.get() << 8) | ((uint16_t) xData.get());
  accelData[1] = ((uint16_t) yData.get() << 8) | ((uint16_t) yData.get());
  accelData[2] = ((uint16_t) zData.get() << 8) | ((uint16_t) zData.get());
}

