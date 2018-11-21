#include "I2CAgent.h"

bool I2CAgent::transmit( Command & cmd )
{
  resetPacket(outPacket_);
  outPacket_.put(writeCommand_);
  cmd.writePacket(outPacket_);
  bool status = driver_->getTransmitter()->run(outPacket_);
  return status;
}

void I2CAgent::setSlaveAddr( uint8_t addr )
{
  slaveAddr_ = addr;
  writeCommand_ = addr << 1;
  readCommand_ = addr << 1 | 1 << 0;
}