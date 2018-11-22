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
#include "I2CAgent.h"
#include "Command.h"
#include "frt_queue.h"

class MB1202
{
public:

	class RangeCommand
		: public Command
	{
	public:
		typedef frt_queue< uint8_t > Packet;
		inline RangeCommand()
			: commandCode_(RANGE_CMD)
		{ }
		void writePacket( Packet & packet );
	protected:
		uint8_t commandCode_;
	};
	
	MB1202 (I2CMaster * d);
	
	bool is_ready(void);
	
	bool take_reading(void);
	
	uint16_t get_reading(void);
	
	void change_slave_addr(uint8_t new_addr);

protected:

	I2CMaster * driver_;
	I2CAgent * i2cAgent_;
	uint8_t slaveAddr_;
	uint8_t range_cmd[1];
	uint8_t addr_change_seq[3] = {0};
	uint8_t bytes_received[2] = {0};
	uint16_t range_reading;
	RangeCommand * rangeCommand_;
	
};


#endif /* MB1202_H_ */