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
MB1202::MB1202(I2CMaster* i2c)
:i2c(i2c)
{
	slave_addr = SLAVE_ADDR;
	range_cmd[0] = RANGE_CMD;
	addr_change_seq[0] = ADDR_CHANGE_0;
	addr_change_seq[1] = ADDR_CHANGE_1;
};

bool MB1202::is_ready (void)
{
	return i2c->is_ready(slave_addr);
};

bool MB1202::take_reading (void)
{
	return i2c->write (slave_addr, range_cmd, 1);
};

uint16_t MB1202::get_reading (void)
{
	i2c->read (slave_addr, bytes_received, 2);
	range_reading = ((uint16_t) bytes_received[0] << 8) | ((uint16_t) bytes_received[1]);
	return range_reading;
}

void MB1202::change_slave_addr(uint8_t new_addr)
{
	addr_change_seq[2] = new_addr << 1;
	i2c->write(slave_addr, addr_change_seq, 3);
}
