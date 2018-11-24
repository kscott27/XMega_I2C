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
}

MMA8451::MMA8451(I2CMaster * d, emstream * s)
  : driver_(d),
    p_serial(s),
    i2cAgent_(new I2CAgent(s, outPacketSize_, inPacketSize_)),
    slaveAddr_(SLAVE_ADDR),
    activeCommand_(new ActiveCommand())
{
  i2cAgent_->setI2CDriver(d);
  i2cAgent_->setSlaveAddr(slaveAddr_);
}

void MMA8451::ActiveCommand::writePacket( Packet & packet )
{
  packet.put(reg_);
  packet.put(data_);
}

bool MMA8451::is_ready()
{
  return driver_->is_ready(slaveAddr_);
}

bool MMA8451::takeReading()
{
  *p_serial << "mb1202 range cmd" << endl;
  return i2cAgent_->transmit(*activeCommand_);
}

uint16_t MMA8451::getReading()
{
  Packet & data = i2cAgent_->receive();
  reading_ = ((uint16_t) data.get() << 8) | ((uint16_t) data.get());
  return reading_;
}

