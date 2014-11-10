#Tiva C Connected Launchpad GPS Data Logger

## GPS Module

[Adafruit Ultimate GPS Breakout](https://www.adafruit.com/products/746)

[Module Datasheet](https://www.adafruit.com/datasheets/GlobalTop-FGPMMOPA6H-Datasheet-V0A.pdf)

[PMTK Command Packet Reference](https://www.adafruit.com/datasheets/PMTK_A11.pdf)

The module used is  based around the MTK3339 chipset. The module outputs NMEA sentences over UART at a default rate of 9600 baud.

## UART

Pin Location  | Port and pin | Function
------------- | -------------|----------
J1-3          | PC4          | RX
J1-4          | PC5          | TX

1. Begin with UART polling version of lab10 as a basic template.

2. To allow for easy debugging, use two UARTs. One UART for GPS input and another that echoes the input to a terminal.
    * Since UART0_BASE is already configured as a PC serial connection, use UART7_BASE for GPS input.
    * Use Port C pins 4 and 5 as GPS UART input.

3. Verify the configuration is correct by building the project and running the code. Open a terminal and verify the NMEA sentences are displaying properly

4. Since code will be reading values into memory, increase the heap and stack size in Code Composer Studio.  I used 0x1000 for both.
