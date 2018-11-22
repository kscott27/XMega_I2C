#ifndef PACKET_H_
#define PACKET_H_

//#define F_CPU 32000000

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/interrupt.h>

#include "frt_queue.h"

class Packet
  : public frt_queue< uint8_t >
{
public:
  inline Packet( uint8_t queue_size = 10, emstream * p_ser_dev = NULL, portTickType wait_time = 10 )
    : frt_queue(queue_size, p_ser_dev, wait_time),
      size_(queue_size)
  { }
  uint8_t getSize() const { return size_; }
protected:
  uint8_t size_;
};

#endif // PACKET_H_