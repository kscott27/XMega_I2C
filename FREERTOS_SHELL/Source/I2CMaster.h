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
#include "frt_queue.h"
#include "Packet.h"
#include "emstream.h"


class I2CMaster
{
public:
  
  class State
  {
  public:
    inline State() 
      : runs_(0)
    { }
    void setTransition( State * nextState, State * returnState )
    {
      nextState_ = nextState;
      returnState_ = returnState;
    }
    virtual State * execute( Packet & packet ) = 0;
    virtual void serialDebug() {}
    void resetRunCount() { runs_ = 0; }

  protected:
    State * nextState_;
    State * returnState_;
    uint8_t runs_;
  };

  class StartState
    : public State
  {
  public:
    StartState( I2CMaster * d )
      : driver_(d)
    { }
    State * execute( Packet & packet );
    void serialDebug() { *(driver_->getSerial()) << "start" << endl; }
  protected:
    I2CMaster * driver_;
  };

  class StatusState
    : public State
  {
  public:
    inline StatusState( I2CMaster * d, uint16_t timeout )
      : driver_(d),
        timeout_(timeout)
    { }
    State * execute( Packet & packet );
    void serialDebug() { *(driver_->getSerial()) << "status" << endl; }
  protected:
    I2CMaster * driver_;
    uint16_t timeout_;
  };

  class DoneState
    : public State
  {
  public:
    inline DoneState( I2CMaster * d )
      : driver_(d)
    { }
    State * execute( Packet & packet );
    void serialDebug() { *(driver_->getSerial()) << "done" << endl; }
  protected:
    I2CMaster * driver_;
  };

  class ErrorState
    : public State
  {
  public:
    inline ErrorState( I2CMaster * d )
      : driver_(d)
    { }
    State * execute( Packet & packet );
    void serialDebug() { *(driver_->getSerial()) << "error" << endl; }
  protected:
    I2CMaster * driver_;
  };

  class Receiver
  {
  public:
    Receiver( I2CMaster * d );
    Packet & run( Packet & packet );

  protected:
    class StatusState
      : public State
    {
    public:
      inline StatusState( I2CMaster * d, uint16_t timeout )
        : driver_(d),
          timeout_(timeout)
      { }
      State * execute( Packet & packet );
      void serialDebug();
    protected:
      I2CMaster * driver_;
      uint16_t timeout_;
    };

    class ExchangeState
      : public State
    {
    public:
      inline ExchangeState( I2CMaster * d )
        : driver_(d)
      { }
      State * execute( Packet & packet );
      void serialDebug() { *(driver_->getSerial()) << "r exchange" << endl; }
    protected:
      I2CMaster * driver_;
    };

    class PacketStatusState
      : public State
    {
    public:
      inline PacketStatusState( I2CMaster * d )
        : driver_(d)
      { }
      State * execute( Packet & packet );
      void serialDebug() { *(driver_->getSerial()) << "r packet" << endl; }
    protected:
      I2CMaster * driver_;
    };

    I2CMaster * driver_;
    State * currentState_;
    State * startState_;
    State * statusState_;
    State * exchangeState_;
    State * packetStatusState_;
    State * errorState_;
    State * doneState_;
    uint8_t slaveAddr_;
    uint16_t timeout_;
    uint8_t packetSize_;
  };

  class Transmitter
  {
  public:
    Transmitter( I2CMaster * d );
    bool run( Packet & packet );

  protected:
    class StatusState
      : public State
    {
    public:
      inline StatusState( I2CMaster * d, uint16_t timeout )
        : driver_(d),
          timeout_(timeout)
      { }
      State * execute( Packet & packet );
      void serialDebug();
    protected:
      I2CMaster * driver_;
      uint16_t timeout_;
    };

    class ExchangeState
      : public State
    {
    public:
      inline ExchangeState( I2CMaster * d )
        : driver_(d)
      { }
      State * execute( Packet & packet );
      void serialDebug() { *(driver_->getSerial()) << "t exchange" << endl; }
    protected:
      I2CMaster * driver_;
    };

    class PacketStatusState
      : public State
    {
    public:
      inline PacketStatusState( I2CMaster * d )
        : driver_(d)
      { }
      State * execute( Packet & packet );
      void serialDebug() { *(driver_->getSerial()) << "t packet" << endl; }
    protected:
      I2CMaster * driver_;
    };

    I2CMaster * driver_;
    State * currentState_;
    State * startState_;
    State * statusState_;
    State * exchangeState_;
    State * packetStatusState_;
    State * errorState_;
    State * doneState_;
    uint8_t slaveAddr_;
    uint16_t timeout_;

  };

  I2CMaster(TWI_t * interface, uint32_t i2c_freq, emstream * s);

  Transmitter * getTransmitter()  { return transmitter_; }
  Receiver *    getReceiver()     { return receiver_; }
  TWI_t *       getInterfacePtr() { return interface_; }
  emstream *    getSerial()       { return p_serial; }
  
  void set_baudrate (uint32_t i2c_freq); 
  uint8_t * scan (void);
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

protected:

  Transmitter * transmitter_;
  Receiver * receiver_;
  TWI_t* interface_;
  emstream * p_serial;
  PORT_t* bus_port;
  uint8_t baudrate;
  uint32_t i2c_freq;
  uint8_t addr_list[10];

};



#endif /* I2CMaster_H_ */
