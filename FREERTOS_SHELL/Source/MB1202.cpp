/*
 * MAX31855.cpp
 *
 * Created: 2/27/2018 11:58:06 PM
 *  Author: Kevin
 */ 

#include "MB1202.h"

//-------------------------------------------------------------------------------------
/** This constructor creates an object to use when interacting with the SPI registers on the MAX31855
    Thermocouple Analog to Digital Converter chip.
 *  @param SPIM A pointer to an SPI_master object
 */
MB1202::MB1202(I2CMaster * d, emstream * s)
  : driver_(d),
  	p_serial(s),
  	i2cAgent_(new I2CAgent(s, outPacketSize_, inPacketSize_)),
  	slaveAddr_(SLAVE_ADDR),
  	rangeCommand_(new RangeCommand())
{
	i2cAgent_->setI2CDriver(d);
	i2cAgent_->setSlaveAddr(slaveAddr_);
}

void MB1202::RangeCommand::writePacket( Packet & packet )
{
	packet.put(commandCode_);
}

bool MB1202::is_ready()
{
	return driver_->is_ready(slaveAddr_);
}

bool MB1202::takeReading()
{
	return i2cAgent_->transmit(*rangeCommand_);
}

uint16_t MB1202::getReading()
{
	Packet & data = i2cAgent_->receive();
  rangeReading_ = 0;
  if( data.validData() )
	  rangeReading_ = ((uint16_t) data.get() << 8) | ((uint16_t) data.get());
	return rangeReading_;
}

void MB1202::change_slave_addr(uint8_t new_addr)
{
	addr_change_seq[2] = new_addr << 1;
	driver_->write(slaveAddr_, addr_change_seq, 3);
}
