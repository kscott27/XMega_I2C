/*
 * I2C.cpp
 *
 * Created: 12/1/2017 12:48:43 AM
 *  Author: Kevin
 */ 

#include "I2CMaster.h"

I2CMaster::I2CMaster(TWI_t * interface, uint32_t i2c_freq, emstream * s)
  : interface_(interface), 
    i2c_freq(i2c_freq),
    p_serial(s)
{ 
  if (interface == &TWIC) {
    bus_port = &PORTC; }
  else if (interface == &TWIE) {
    bus_port = &PORTE; }
  
  bus_port->DIRSET = PIN0_bm | PIN1_bm;
  bus_port->PIN0CTRL = PORT_OPC_WIREDANDPULL_gc; //SDA pull up output
  bus_port->PIN1CTRL = PORT_OPC_WIREDANDPULL_gc; //SCL pull up output
  // Set the quick command enable bit so that status interrupt flags are set immediately after slave ACKs
  interface->MASTER.CTRLB = 1 << 1;
  set_baudrate(i2c_freq); //baud rate is set such that TWI freq=100KHz
  interface->MASTER.STATUS |= TWI_MASTER_RIF_bm | TWI_MASTER_WIF_bm | TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm | TWI_MASTER_BUSSTATE_IDLE_gc; //clear all flags initially and select bus state IDLE
  interface->MASTER.CTRLA = TWI_MASTER_ENABLE_bm;

  transmitter_ = new Transmitter(this);
  receiver_ = new Receiver(this);

  *s << "I2CMaster created" << endl;
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
  
  while( (currentState_ != doneState_) && (currentState_ != errorState_) ) {
    currentState_ = currentState_->execute(packet); }

  currentState_->execute(packet);

  if( currentState_ == doneState_ ) {
    return true; }
  else {
    return false; }
}

void I2CMaster::Transmitter::StatusState::serialDebug()
{
  uint8_t status = driver_->getInterfacePtr()->MASTER.STATUS;
  *(driver_->getSerial()) << "T Status: " << status << endl;
}

void I2CMaster::Receiver::StatusState::serialDebug()
{
  uint8_t status = driver_->getInterfacePtr()->MASTER.STATUS;
  *(driver_->getSerial()) << "R Status: " << status << endl;
}

Packet & I2CMaster::Receiver::run( Packet & packet )
{
  currentState_ = startState_;
  
  while( (currentState_ != doneState_) && (currentState_ != errorState_) ) {
    currentState_ = currentState_->execute(packet); }

  currentState_->execute(packet);

  return packet;
}

I2CMaster::State * I2CMaster::StartState::execute( Packet & packet )
{
  uint8_t * startCommand;
  packet.get(startCommand);
  driver_->send_start();
  driver_->getInterfacePtr()->MASTER.ADDR = *startCommand;
  return nextState_;
}

I2CMaster::State * I2CMaster::Transmitter::StatusState::execute( Packet & packet )
{
  volatile uint16_t counter;
  counter = timeout_;
  while( ( --counter != 0 ) && 
         !( driver_->getInterfacePtr()->MASTER.STATUS & TWI_MASTER_WIF_bm ) ) { }
  if( !(driver_->getInterfacePtr()->MASTER.STATUS & TWI_MASTER_RXACK_bm) &&
        (driver_->getInterfacePtr()->MASTER.STATUS & TWI_MASTER_WIF_bm) ) {
    return nextState_; }
  else {
    return returnState_; }
}

I2CMaster::State * I2CMaster::Receiver::StatusState::execute( Packet & packet )
{
  volatile uint16_t counter;
  counter = timeout_;
  while( ( --counter != 0) && 
         !( driver_->getInterfacePtr()->MASTER.STATUS & TWI_MASTER_RIF_bm ) ) { }
  if( !(driver_->getInterfacePtr()->MASTER.STATUS & TWI_MASTER_RXACK_bm) &&
        (driver_->getInterfacePtr()->MASTER.STATUS & TWI_MASTER_RIF_bm) ) {
    return nextState_; }
  else {
    return returnState_; }
}

I2CMaster::State * I2CMaster::Transmitter::ExchangeState::execute( Packet & packet )
{
  if( packet.not_empty() ) {
    uint8_t * data; 
    packet.get(data);
    driver_->getInterfacePtr()->MASTER.DATA = *data; }
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
  if( packet.is_empty() ) {
    return nextState_; }
  else {
    return returnState_; }
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
  packet.isValid();
  return nextState_;
}

I2CMaster::State * I2CMaster::ErrorState::execute( Packet & packet )
{
  uint8_t statusReg = driver_->getInterfacePtr()->MASTER.STATUS;
  *(driver_->getSerial()) << "I2C transfer failed with status: " << statusReg << endl;
  return nextState_;
}

void I2CMaster::set_baudrate(uint32_t i2c_freq)
{
  baudrate = (F_CPU / (2 * i2c_freq)) - 5;
  interface_->MASTER.BAUD = baudrate;
}

uint8_t * I2CMaster::scan (void)
{
  volatile uint16_t counter;
  
  uint8_t addr_list_index;
  addr_list_index = 0;
  
  for (uint8_t addr = 1; addr < 128; addr++)
  {
    counter = 100;
    send_start();
    interface_->MASTER.ADDR = addr << 1;
    
    while ((--counter != 0) && (interface_->MASTER.STATUS & TWI_MASTER_RXACK_bm)){ }
    if(counter != 0)
    {
      uint8_t addr_reg = interface_->MASTER.ADDR;
      *p_serial << "Detected i2c addr: " << addr_reg << endl;
      addr_list[addr_list_index] = addr;
      addr_list_index++;
    }
    send_nack_stop();    
  }
  return addr_list;
}

bool I2CMaster::is_ready (uint8_t addr)
{
  volatile uint16_t counter = 500;
  send_start();
  interface_->MASTER.ADDR = addr << 1;
  
  while( (--counter != 0) && (interface_->MASTER.STATUS & TWI_MASTER_RXACK_bm) ) { }
  if(counter != 0) {
    return true; }
  else {
    return false; }

  send_nack_stop();
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
