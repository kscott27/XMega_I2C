#include "I2CAgent.h"

bool I2CAgent::transmit( Command & cmd )
{
  writePacket(cmd);
  bool status = driver_->getTransmitter()->run(outPacket_);
  return status;
}

// I2CAgent::Packet I2CAgent::receive()
// {
//   resetPacket(inPacket_);
//   inPacket_.put(readCommand_);
//   // return driver_->getReceiver()->run(inPacket_);
//   return Packet packet(10, NULL, 10);
// }

void I2CAgent::setSlaveAddr( uint8_t addr )
{
  slaveAddr_ = addr;
  writeCommand_ = addr << 1;
  readCommand_ = addr << 1 | 1 << 0;
}

void I2CAgent::writePacket( Command & cmd )
{
  resetPacket(outPacket_);
  outPacket_.put(writeCommand_);
  cmd.writePacket(outPacket_);
}

void I2CAgent::readPacket()
{
  resetPacket(inPacket_);
}

void I2CAgent::resetPacket( Packet & packet )
{
  while( packet.not_empty() )
  {
    packet.get();
  }
}
