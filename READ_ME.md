## XMega I2C Driver and Associated Files

This repository is set up to run an Atmel Studio solution on an XMega128A4U. It will produce serial output of sensor readings if connected to an MMA8451 accelerometer.

I2CMaster Class
---
The I2CMaster class is the core driver of the I2C bus, and encapsulates all of the low-level register logic needed to transfer data along the bus. It utilizes two member classes, a Transmitter and Receiver class, in order to handle the two directional communication modes. The Transmitter and Receiver each implement their own state machine logic to handle the proper commands with the proper commands based on reading and writing to the registers of the TWI module.

I2CAgent Class
---
The I2CAgent class is meant to prepare I2C packets in the proper format for the I2CMaster class. Any device driver class that needs to interface with the I2CMaster should create and implement its own I2CAgent. Since each device on the I2C bus should have its own Agent, each Agent should have a specific I2C address that it is tending to. The I2C address should be set using the setSlaveAddr() method. Ideally, this method should be called in the constructor of the device driver, after the I2CAgent has been constructed. The I2CAgent is basically the API for interacting with the I2CMaster - a developer trying to use the I2CMaster should never need to reference methods of the I2CMaster class, and should instead simply call methods from the I2CAgent. An example of this can be seen by observing the way the I2CAgent is utilized within the MMA8451 class.

The I2CAgent inherits from the CommAgent class. The CommAgent is a pure virtual interface, meaning that it cannot be instantiated. The reason for this structure is so that other types of communication agents could also inherit from the CommAgent - such as SPI or UART drivers. Then, a device driver could have the ability to call any type of CommAgent through a CommAgent pointer (known as dynamic or run-time binding). This sort of binding allows for more versatile code - the same driver could use a CommAgent pointer to implement either I2C or some other form of peripheral communication, based on what specific type of child object the base class pointer has received.

MMA8451 Class
---
The MMA8451 class encapsulates all of the details needed to operate that specific accelerometer over I2C. It handles the specific data that needs to be put in packets and stuffed down to the I2CMaster. This data entails the specific registers and bits within those registers that need to be written to and read from to get the sensor into the proper mode and then access the desired data. The different commands are defined in this class, and then handed over to the I2CAgent so that they can be properly placed into Packets in the proper format. The I2CAgent then hands the packet down to the I2CMaster for bus interaction.

Packet Class
---
The Packet class is really just a fancy queue. It inherits from an frt_queue of uint8_t. It then stores specifc data such as the size of the packet, and whether the data in the packet is valid (packet data is only declared valid once the I2CMaster has reached the "DoneState"). The Packet class also has its own methods for manipulating this data and emptying the queue of its contents every time a new transaction is initiated. The I2CAgent is built to construct two Packets - an inPacket and outPacket. The contents of these packets is reset as each transfer is initiated. The Packet header file should be included in device driver files, because the I2CAgent will return data from the I2C bus in Packet form, so the device class will need to be aware of the details of Packets.

