#TM4C1294 Connected LaunchPad GPS Data Logger
##### TM4C1294NCPDT MCU: 120MHz 32-bit ARM Cortex-M4

[Schematics and PCB Layout](https://upverter.com/mitchg45/cc48ca266b1d3310/GPS-Logger-Boosterpack-80-Pin/)

##Pin Connections

Module  |   Connector (Pin) | Port and Pin | Peripheral | Function
--------|-------------------|--------------|------------|---------
GPS     | X8 (J1-3)         | PortC Pin 4  | UART7      | RX
GPS     | X8 (J1-4)         | PortC Pin 5  | UART7      | TX
GPS     | X6 (J3-67)        | PortK Pin 2  | GPIO       | PPS (Pulse Per Second)
MicroSD | X8 (J1-7)         | PortD Pin 3  | SSI2       | CLK (Clock)
MicroSD | X9 (J2-15)        | PortD Pin 1  | SSI2       | MOSI (Master Out - Slave In)
MicroSD | X9 (J2-14)        | PortD Pin 0  | SSI2       | MISO (Master In - Slave Out)
MicroSD | X6 (J1-42)        | PortD Pin 2  | SSI2       | CS (Chip Select)
MicroSD | X6 (J3-68)        | PortK Pin 3  | GPIO       | CD (Card Detect)
