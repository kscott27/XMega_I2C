/*
 * I2C.cpp
 *
 * Created: 12/1/2017 12:48:43 AM
 *  Author: Kevin
 */ 

#include "I2CMaster.h"

I2CMaster::I2CMaster(TWI_t * interface, uint32_t i2c_freq)
  : interface_(interface), 
    i2c_freq(i2c_freq),
    transmitter_(new Transmitter(this)),
    receiver_(new Receiver(this))
{ 
  if (interface == &TWIC)
  {
    bus_port = &PORTC;
  }
  if (interface == &TWIE)
  {
    bus_port = &PORTE;
  }
  
  bus_port->DIRSET = PIN0_bm | PIN1_bm;
  bus_port->PIN0CTRL = PORT_OPC_WIREDANDPULL_gc; //SDA pull up output
  bus_port->PIN1CTRL = PORT_OPC_WIREDANDPULL_gc; //SCL pull up output
  
  interface->MASTER.CTRLB = 1 << 1;
  
  set_baudrate(i2c_freq); //baud rate is set such that TWI freq=100KHz
  
//  ptwiport->CTRL=0x00; //SDA hold time off, normal TWI operation
  
//  ptwiport->MASTER.CTRLA = TWI_MASTER_INTLVL_HI_gc|TWI_MASTER_RIEN_bm|TWI_MASTER_WIEN_bm|TWI_MASTER_ENABLE_bm; //enable high priority read and write interrupt, enable MASTER
  
//  ptwiport->MASTER.CTRLB = 0x00; //TWI_MASTER_QCEN_bm; //no inactive bus timeout, quick command and smart mode enabled
  
//  ptwiport->MASTER.CTRLC = 0x00; //initially send ACK and no CMD selected

  interface->MASTER.STATUS |= TWI_MASTER_RIF_bm | TWI_MASTER_WIF_bm | TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm | TWI_MASTER_BUSSTATE_IDLE_gc; //clear all flags initially and select bus state IDLE

  interface->MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
}

I2CMaster::I2CMaster(TWI_t * interface, uint32_t i2c_freq, emstream * s)
  : interface_(interface), 
    i2c_freq(i2c_freq),
    transmitter_(new Transmitter(this)),
    receiver_(new Receiver(this)),
    p_serial(s)
{ 
  if (interface == &TWIC)
  {
    bus_port = &PORTC;
  }
  if (interface == &TWIE)
  {
    bus_port = &PORTE;
  }
  
  bus_port->DIRSET = PIN0_bm | PIN1_bm;
  bus_port->PIN0CTRL = PORT_OPC_WIREDANDPULL_gc; //SDA pull up output
  bus_port->PIN1CTRL = PORT_OPC_WIREDANDPULL_gc; //SCL pull up output
  
  interface->MASTER.CTRLB = 1 << 1;
  
  set_baudrate(i2c_freq); //baud rate is set such that TWI freq=100KHz
  
//  ptwiport->CTRL=0x00; //SDA hold time off, normal TWI operation
  
//  ptwiport->MASTER.CTRLA = TWI_MASTER_INTLVL_HI_gc|TWI_MASTER_RIEN_bm|TWI_MASTER_WIEN_bm|TWI_MASTER_ENABLE_bm; //enable high priority read and write interrupt, enable MASTER
  
//  ptwiport->MASTER.CTRLB = 0x00; //TWI_MASTER_QCEN_bm; //no inactive bus timeout, quick command and smart mode enabled
  
//  ptwiport->MASTER.CTRLC = 0x00; //initially send ACK and no CMD selected

  interface->MASTER.STATUS |= TWI_MASTER_RIF_bm | TWI_MASTER_WIF_bm | TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm | TWI_MASTER_BUSSTATE_IDLE_gc; //clear all flags initially and select bus state IDLE

  interface->MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
}

void I2CMaster::scanBus()
{

}

I2CMaster::Transmitter::Transmitter( I2CMaster * d )
  : driver_(d),
    timeout_(10000),
    startState_(new StartState( d )),
    statusState_(new StatusState( d, timeout_ )),
    exchangeState_(new ExchangeState( d )),
    packetStatusState_(new PacketStatusState( d )),
    doneState_(new DoneState( d )),
    errorState_(new ErrorState( d ))
{
  startState_->setTransition(statusState_, statusState_);
  statusState_->setTransition(exchangeState_, errorState_);
  exchangeState_->setTransition(packetStatusState_, packetStatusState_);
  packetStatusState_->setTransition(doneState_, statusState_);
}

I2CMaster::Receiver::Receiver( I2CMaster * d )
  : driver_(d),
    timeout_(10000),
    startState_(new StartState( d )),
    statusState_(new StatusState( d, timeout_ )),
    exchangeState_(new ExchangeState( d )),
    packetStatusState_(new PacketStatusState( d )),
    doneState_(new DoneState( d )),
    errorState_(new ErrorState( d ))
{
  startState_->setTransition(statusState_, statusState_);
  statusState_->setTransition(exchangeState_, errorState_);
  exchangeState_->setTransition(packetStatusState_, packetStatusState_);
  packetStatusState_->setTransition(doneState_, statusState_);
}

bool I2CMaster::Transmitter::run( Packet & packet )
{
  currentState_ = startState_;
  
  while( (currentState_ != doneState_) || (currentState_ != errorState_) )
  {
    State * transitionState = currentState_->execute(packet);
    currentState_->serialDebug();
    currentState_ = transitionState;
  }

  currentState_->serialDebug();

  if( currentState_ == doneState_ )
  {
    return true;
  }
  else
  {
    return false;
  }
}

Packet & I2CMaster::Receiver::run( Packet & packet )
{
  currentState_ = startState_;
  
  while( (currentState_ != doneState_) || (currentState_ != errorState_) )
  {
    State * transitionState = currentState_->execute(packet);
    currentState_->serialDebug();
    currentState_ = transitionState;
  }

  currentState_->serialDebug();

  return packet;
}

I2CMaster::State * I2CMaster::StartState::execute( Packet & packet )
{
  uint8_t * startCommand;
  packet.get(startCommand);
  send_start();
  driver_->getInterfacePtr()->MASTER.ADDR = *startCommand;
  return nextState_;
}

I2CMaster::State * I2CMaster::Transmitter::StatusState::execute( Packet & packet )
{
  volatile uint16_t counter;
  counter = timeout_;
  // Need to figure out which status bit to be checking
  while( ( --counter != 0 ) && 
         ( !(driver_->getInterfacePtr()->MASTER.STATUS) & TWI_MASTER_WIF_bm ) ) { }
  if( (!(driver_->getInterfacePtr()->MASTER.STATUS) & TWI_MASTER_RXACK_bm) &&
        (driver_->getInterfacePtr()->MASTER.STATUS & TWI_MASTER_WIF_bm) )
  {
    return nextState_;
  }
  else
  {
    return returnState_;
  }
}

I2CMaster::State * I2CMaster::Receiver::StatusState::execute( Packet & packet )
{
  volatile uint16_t counter;
  counter = timeout_;
  // Need to figure out which status bit to be checking
  while( ( --counter != 0) && 
         ( !(driver_->getInterfacePtr()->MASTER.STATUS) & TWI_MASTER_RIF_bm ) ){ }
  if( (!(driver_->getInterfacePtr()->MASTER.STATUS) & TWI_MASTER_RXACK_bm) &&
        (driver_->getInterfacePtr()->MASTER.STATUS & TWI_MASTER_RIF_bm) ) {
    return nextState_; }
  else {
    return returnState_; }
}

I2CMaster::State * I2CMaster::Transmitter::ExchangeState::execute( Packet & packet )
{
  if( packet.not_empty() )
  {
    uint8_t * data; 
    packet.get(data);
    driver_->getInterfacePtr()->MASTER.DATA = *data;
  }
  return nextState_;
}

I2CMaster::State * I2CMaster::Receiver::ExchangeState::execute( Packet & packet )
{
  uint8_t data = driver_->getInterfacePtr()->MASTER.DATA;
  packet.put(data);
  return nextState_;
}

I2CMaster::State * I2CMaster::Transmitter::PacketStatusState::execute( Packet & packet )
{
  if( packet.is_empty() )
  {
    return nextState_;
  }
  else
  {
    return returnState_;
  }
}

I2CMaster::State * I2CMaster::Receiver::PacketStatusState::execute( Packet & packet )
{
  if( packet.num_items_in() == packet.getSize() ) {
    driver_->send_nack_stop();
    return nextState_; }
  else {
    driver_->byte_recv();
    return returnState_; }
}

I2CMaster::State * I2CMaster::DoneState::execute( Packet & packet )
{
  return nextState_;
}

I2CMaster::State * I2CMaster::ErrorState::execute( Packet & packet )
{
  return nextState_;
}

void I2CMaster::set_baudrate(uint32_t i2c_freq)
{
  baudrate = (F_CPU / (2 * i2c_freq)) - 5;
  interface_->MASTER.BAUD = baudrate;
}

uint8_t* I2CMaster::scan (void)
{
  volatile uint16_t counter;
  
  uint8_t addr_list_index;
  addr_list_index = 0;
  
  for (uint8_t addr = 0; addr < 128; addr++)
  {
    counter = 100;
    interface_->MASTER.ADDR = addr << 1;
    
    while ((--counter != 0) && (interface_->MASTER.STATUS & TWI_MASTER_RXACK_bm)){ }
    if(counter != 0)
    {
      addr_list[addr_list_index] = addr;
      addr_list_index++;
    }
      
  }
  
  return addr_list;
}

bool I2CMaster::is_ready (uint8_t addr)
{
  volatile uint16_t counter = 500;
  
  interface_->MASTER.ADDR = addr << 1;
  
  while ((--counter != 0) && (interface_->MASTER.STATUS & TWI_MASTER_RXACK_bm)){ }
  if(counter != 0)
  {
    send_nack_stop();
    return true;
  }
  else
  {
    return false;
  }
}

bool I2CMaster::write (uint8_t slave_addr, uint8_t* data, uint8_t packet_len, uint16_t timeout)
{
  volatile uint16_t counter;
  counter = timeout;
  //data_out = data;
  
  interface_->MASTER.ADDR = slave_addr << 1;
  
  while ((--counter != 0) && !(interface_->MASTER.STATUS & TWI_MASTER_WIF_bm)){ }
  if (counter != 0)
  {
    for (uint8_t i = 0; i < packet_len; i++)
    {
      counter = timeout;
      interface_->MASTER.DATA = data[i];
      while ((--counter != 0) && (interface_->MASTER.STATUS & TWI_MASTER_RXACK_bm)){ }
      if (counter == 0)
      {
        return false;
      }
      else
      {
        interface_->MASTER.STATUS |= TWI_MASTER_WIF_bm;
      }
    }
    //interface_->MASTER.STATUS |= 1 << 6;
    send_stop();
    return true;  
  }
  else
  {
    return false;
  }
}

bool I2CMaster::mem_write (uint8_t slave_addr, uint8_t mem_addr, uint8_t* data, uint8_t packet_len, uint16_t timeout)
{
  volatile uint16_t counter;
  counter = timeout;
  
  interface_->MASTER.ADDR = slave_addr << 1;
  
  while ((--counter != 0) && (interface_->MASTER.STATUS & TWI_MASTER_RXACK_bm)){ }
  if (counter != 0)
  {
    interface_->MASTER.DATA = mem_addr;
    
    while ((--counter != 0) && (interface_->MASTER.STATUS & TWI_MASTER_RXACK_bm)){ }
    if (counter != 0)
    {
      for (uint8_t i = 0; i < packet_len; i++)
      {
        counter = timeout;
        interface_->MASTER.DATA = data[i];
        while ((--counter != 0) && (interface_->MASTER.STATUS & TWI_MASTER_RXACK_bm)){ }
        if (counter == 0)
        {
          return false;
        }
      }
      send_stop();
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool I2CMaster::read (uint8_t slave_addr, uint8_t* data, uint8_t packet_len, uint16_t timeout)
{
  volatile uint16_t counter;
  volatile uint8_t i;
  counter = timeout;
  
  interface_->MASTER.ADDR = slave_addr << 1 | 1 << 0;
  
  while ((--counter != 0) && !(interface_->MASTER.STATUS & TWI_MASTER_RIF_bm)){ }
  if (counter != 0)
  {
    for (i = 0; i < packet_len; ++i)
    {
      counter = timeout;
      while ((--counter != 0) && !(interface_->MASTER.STATUS & TWI_MASTER_RIF_bm)){ }      
      if (counter != 0)
      {
        data[i] = interface_->MASTER.DATA;
        if (i < packet_len)
        {
          byte_recv();
          interface_->MASTER.STATUS |= TWI_MASTER_RIF_bm;
        }
        else
        {
          send_nack_stop();
          interface_->MASTER.STATUS |= TWI_MASTER_RIF_bm;
          return true;
        }
      }
      else
      {
        return false;
      }   
    }
  }
  else
  {
    return false;
  }
}


void I2CMaster::send_start(void)
{
  interface_->MASTER.CTRLC |= (1<<TWI_MASTER_CMD0_bp);
}

void I2CMaster::byte_recv(void)
{
  interface_->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
}

void I2CMaster::send_ack()
{
  //interface_->MASTER.CTRLC = interface_->MASTER.STATUS & ~(TWI_MASTER_RXACK_bm);
  interface_->MASTER.CTRLC = 0x00;
}

void I2CMaster::send_rep_start(void)
{
  interface_->MASTER.CTRLC |= TWI_MASTER_CMD_REPSTART_gc;
}

void I2CMaster::send_nack_stop(void)
{
  interface_->MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
}

void I2CMaster::send_ack_stop(void)
{
  interface_->MASTER.CTRLC = (1<<TWI_MASTER_CMD1_bp) | (1<<TWI_MASTER_CMD0_bp);
}

void I2CMaster::send_stop(void)
{
  interface_->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}
