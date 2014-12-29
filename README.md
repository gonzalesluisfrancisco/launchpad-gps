#TM4C1294 Connected LaunchPad GPS Data Logger
##### TM4C1294NCPDT MCU: 120MHz 32-bit ARM Cortex-M4

[Schematics and PCB Layout](https://upverter.com/mitchg45/cc48ca266b1d3310/GPS-Logger-Boosterpack-80-Pin/)

##Overview

This project combines the Texas Instruments [TM4C1294 Connected LaunchPad](http://www.ti.com/tool/ek-tm4c1294xl) with the [Ultimate GPS Breakout](https://www.adafruit.com/products/746) and [MicroSD card breakout board+](https://www.adafruit.com/product/254), both from from Adafruit Industries. In its current form, the unit logs the following data to a microSD card as a TSV file titled "GPS_LOG.txt".
* Date
* Time (UTC)
* Coordinates (in decimal degrees)
* Speed (mph)
* Course

The unit defaults to Low Power Mode when powered on or reset via the Reset button. This mode logs the aforementioned data once every 10 minutes. Normal Mode can be entered by pressing the Wake button. While in Normal Mode, the data is logged once per second. Additionally, while in Normal Mode the data can be viewed by in a serial terminal (115200 baud 8-N-1) by connecting to the Debug microUSB.

This project is still under active development including the Schematics and PCB Layout. 

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
