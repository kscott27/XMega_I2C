/*
 * I2C.h
 *
 * Created: 12/1/2017 12:06:55 AM
 *  Author: Kevin
 */ 


#ifndef I2CMaster_H_
#define I2CMaster_H_

//#define F_CPU 32000000

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/interrupt.h>


class I2CMaster
{
	protected:
	
	TWI_t* interface;
	PORT_t* bus_port;
	uint8_t baudrate;
	uint32_t i2c_freq;
	//uint8_t* data_out;
	
	uint8_t addr_list[10];

	class State
	{
  public:
    inline State() { }
    void setTransition( State * nextState, State * errorState )
    {
      nextState_ = nextState;
      errorState_ = errorState;
    }
    virtual State * execute( uint8_t & packet ) = 0;

  protected:
    State * nextState_;
    State * errorState_;
	};

  class Transmitter
  {
  public:
    Transmitter( TWI_t * interface );
    void run( uint8_t & packet );
  protected:

    class StartState
      : public State
    {
    public:
      inline StartState( TWI_t * interface )
        : interface_(interface)
      { }
      State * execute( uint8_t & packet );
    protected:
      TWI_t * interface_;
    };

    class StatusState
      : public State
    {
    public:
      inline StatusState( TWI_t * interface, uint16_t timeout )
        : interface_(interface),
          timeout_(timeout)
      { }
      State * execute( uint8_t & packet );
    protected:
      TWI_t * interface_;
      uint16_t timeout_;
    };

    class DoneState
      : public State
    {
    public:
      inline DoneState( TWI_t * interface )
        : interface_(interface)
      { }
      State * execute( uint8_t & packet );
    protected:
      TWI_t * interface_;
    };

    class ErrorState
      : public State
    {
    public:
      inline ErrorState( TWI_t * interface )
        : interface_(interface)
      { }
      State * execute( uint8_t & packet );
    protected:
      TWI_t * interface_;
    };

    State * currentState_;
    State * startState_;
    State * statusState_;
    State * errorState_;
    State * doneState_;
    TWI_t * interface_;
    uint8_t slaveAddr_;
    uint16_t timeout_;

  };


  Transmitter * transmitter_;
	
	public:
	
	I2CMaster(TWI_t * interface, uint32_t i2c_freq);

  Transmitter * getTransmitter() { return transmitter_; }
	
	void set_baudrate (uint32_t i2c_freq);
	
	uint8_t* scan (void);
	
	bool is_ready (uint8_t addr);
	
	bool write (uint8_t slave_addr, uint8_t* data, uint8_t packet_len, uint16_t timeout=1000);
	
	bool mem_write (uint8_t slave_addr, uint8_t mem_addr, uint8_t* data, uint8_t packet_len, uint16_t timeout=100000);
	
	bool read (uint8_t slave_addr, uint8_t* data, uint8_t packet_len, uint16_t timeout=1000);
	
	bool mem_read (uint8_t slave_addr, uint8_t mem_addr, uint8_t* data, uint8_t packet_len, uint16_t timeout=100000);
	
	void I2CInitWrite(void);
	
	void I2CInitRead(void);
	
	void I2CWriteDataTrans(void);
	
	void I2CReadDataTrans(void);
	
	void I2CAckStop(void);
	
	void send_start(void);
	
	void byte_recv(void);
	
	void send_ack(void);
	
	void send_rep_start(void);
	
	void send_nack_stop(void);
	
	void send_ack_stop(void);
	
	void send_stop(void);
};



#endif /* I2CMaster_H_ */

