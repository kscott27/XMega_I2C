/*
 * MAX31855.h
 *
 * Created: 2/27/2018 11:57:36 PM
 *  Author: Kevin
 */ 


#ifndef MB1202_H_
#define MB1202_H_

#define SLAVE_ADDR 112
#define RANGE_CMD 81
#define ADDR_CHANGE_0 170
#define ADDR_CHANGE_1 165

#include "I2CMaster.h"

class MB1202
{
	protected:
	
	I2CMaster* i2c;
	uint8_t slave_addr;
	uint8_t range_cmd[1];
	uint8_t addr_change_seq[3] = {0};
	uint8_t bytes_received[2] = {0};
	uint16_t range_reading;

	public:
	
	MB1202 (I2CMaster* i2c);
	
	bool is_ready(void);
	
	bool take_reading(void);
	
	uint16_t get_reading(void);
	
	void change_slave_addr(uint8_t new_addr);
	
};


#endif /* MB1202_H_ */