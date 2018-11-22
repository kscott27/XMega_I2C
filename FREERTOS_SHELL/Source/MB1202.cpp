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
MB1202::MB1202(I2CMaster * d)
  : driver_(d),
  	i2cAgent_(new I2CAgent()),
  	slaveAddr_(SLAVE_ADDR),
  	rangeCommand_(new RangeCommand())
{
	i2cAgent_->setI2CDriver(d);
	i2cAgent_->setSlaveAddr(slaveAddr_);
	range_cmd[0] = RANGE_CMD;
	addr_change_seq[0] = ADDR_CHANGE_0;
	addr_change_seq[1] = ADDR_CHANGE_1;
}

void MB1202::RangeCommand::writePacket( Packet & packet )
{
	packet.put(commandCode_);
}

bool MB1202::is_ready (void)
{
	return driver_->is_ready(slaveAddr_);
}

bool MB1202::take_reading (void)
{
	return driver_->write (slaveAddr_, range_cmd, 1);
}

uint16_t MB1202::get_reading (void)
{
	driver_->read (slaveAddr_, bytes_received, 2);
	range_reading = ((uint16_t) bytes_received[0] << 8) | ((uint16_t) bytes_received[1]);
	return range_reading;
}

void MB1202::change_slave_addr(uint8_t new_addr)
{
	addr_change_seq[2] = new_addr << 1;
	driver_->write(slaveAddr_, addr_change_seq, 3);
}
