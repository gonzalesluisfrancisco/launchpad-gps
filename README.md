#Tiva C Connected Launchpad GPS Data Logger

[Demo Video on Youtube](http://youtu.be/Iqw6D-W27hI)

## GPS Module

[Adafruit Ultimate GPS Breakout](https://www.adafruit.com/products/746)

[Module Datasheet](https://www.adafruit.com/datasheets/GlobalTop-FGPMMOPA6H-Datasheet-V0A.pdf)

[PMTK Command Packet Reference](https://www.adafruit.com/datasheets/PMTK_A11.pdf)

##Pin Connections
Module        |   Connector-Pin | Port and Pin | Function
------------------------------------------------------------
GPS UART      | X1-21           | PortC Pin 4  | RX
GPS UART      | X1-23           | PortC Pin 5  | TX
GPS UART      | X1-30           | PortK Pin 2  | PPS (Pulse Per Second)
MicroSD Card  | X1-46           | PortD        | CLK (Clock)
MicroSD Card  | X1-48           | PortD Pin 1  | MOSI (Master Out - Slave In)
MicroSD Card  | X1-50           | PortD Pin 0  | MISO (Master In - Slave Out)
MicroSD Card  | X1-52           | PortD Pin 2  | CS (Chip Select)
MicroSD Card  | X1-32           | PortK Pin 3  | CD (Card Detect)