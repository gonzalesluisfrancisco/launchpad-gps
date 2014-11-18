#Tiva C Connected Launchpad GPS Data Logger

[Demo Video on Youtube](http://youtu.be/Iqw6D-W27hI)

## GPS Module

[Adafruit Ultimate GPS Breakout](https://www.adafruit.com/products/746)

[Module Datasheet](https://www.adafruit.com/datasheets/GlobalTop-FGPMMOPA6H-Datasheet-V0A.pdf)

[PMTK Command Packet Reference](https://www.adafruit.com/datasheets/PMTK_A11.pdf)

The module used is based around the MTK3339 chipset. The module outputs NMEA sentences over UART at a default rate of 9600 baud with 1 Hz updates.

## UART

Pin Location  | Port and pin | Function
------------- | -------------|----------
J1-3          | PC4          | RX
J1-4          | PC5          | TX

1. Begin with UART polling version of lab10 from [TM4C1294XL Connected LaunchPad Workshop](http://processors.wiki.ti.com/index.php/Creating_IoT_Solutions_with_the_TM4C1294XL_Connected_LaunchPad_Workshop) as a basic template.
2. To allow for easy debugging, use two UARTs. One UART for GPS input and another that echoes the input to a terminal.
    * Since UART0_BASE is already configured as a PC serial connection, use UART7_BASE for GPS input.
    * Use Port C pins 4 and 5 as GPS UART input.

3. Verify the configuration is correct by building the project and running the code. Open a terminal and verify the NMEA sentences are displaying properly

4. Since code will be reading values into memory, increase the heap and stack size in Code Composer Studio. I used 0x1000 for both.

## GPS Data Parsing

The GPS module ouputs five comma delimited NMEA sentences containing the data to be parsed: GGA, GSA, GSV, RMC, and VTG. Each sentence begins with "$GP" prepended to the sentence. For example, `$GPRMC,064951.000,A,2307.1256,N,12016.4438,E,0.03,1 65.48,260406,3.05,W,A*2C`

#### GPRMC Sentence Structure

Name                  | Example    | Units   |  Description
----------------------|------------|---------|--------------
Message ID            | $GPRMC     |         | RMC protocol header
UTC Time              | 064951.000 |         | hhmmss.sss
Status                | A          |         | A=data valid or V=data not valid
Latitude              | 2307.1256  |         | ddmm.mmmm
N/S Indicator         | N          |         | N=north or S=south
Longitude             | 12016.4438 |         | dddmm.mmmm
E/W Indicator         | E          |         | E=east or W=west
Speed over Ground     | 0.03       | knots   |
Course over Ground    | 165.48     | degrees | True
Date                  | 260406     |         | ddmmyy
Magnetic Variation    | 3.05, W    |         | degrees E=east or W=west (Need GlobalTop Customization Service)
Mode                  | A          |         | A=Autonomous mode; D=Differential mode; E=Estimated mode
Checksum              |            |         | *2C
&lt;CR&gt; &lt;LF&gt; |            |         | End of message termination

#### String to Float Conversion
Call during config `FPUEnable()` and `FPULazyStackingEnable()` to enable floating-point operations.

A string to floating point conversion function, `ustrtof()`, is included in ustdlib.h which is located in the Tivaware utils folder.  Add the ustdlib.c file to the project as a symbolic link to use these functions.

#### Coordinate Conversion

The latitude and longitude are formatted as degrees and minutes (ddmm.mmmm). However, many popular mapping utilities such as Google Maps do not support this format. The following formula can be used to convert to decimal format either during the logging or when the data is uploaded to another source for analysis computer.

dd = degrees
mm.mmmm = minutes

decimal = dd + (mm.mmmm/60)


#### SD Card Data Logging

Pin Location  | Port and pin | Function
------------- | -------------|----------
X1-46         | PD3          | CLK
X1-48         | PD1          | MOSI
X1-50         | PD0          | MISO
X1-52         | PD2          | CS

Tivaware makes use of the [FatFs](http://elm-chan.org/fsw/ff/00index_e.html) Generic FAT File System Module library and is included in ${TIVAWARE_INSTALL}/third_party/fatfs which contains a port to the Tiva C Connected Launchpad located at port/mmc-ek-tm4c1294xl.c. The functions required for SD access are defined per the [FatFs Application Notes](http://elm-chan.org/fsw/ff/en/appnote.html).  An example program for the dk-tm4c129x located at `${TIVAWARE_INSTALL}/examples/boards/dk-tm4c129x/sd_card/dk-tm4c129x` development board was used in adapting the code for the project.

1. Add the required header files `#include "third_party/src/fatfs/ff.h"` and `#include "third_party/fatfs/src/diskio.h"`.
2. Link `${TIVAWARE_INSTALL}/third_party/fatfs/src/ff.c`
3. Link the ported FatFs file `${TIVAWARE_INSTALL}/third_party/fatfs/port/mmc-ek-tm4c1294xl.c`
    * In this file, define the MCU that is being used for use by the ROM functions included in the file with `#define TARGET_IS_TM4C129_RA1` or `#define TARGET_IS_TM4C129_RA0`. Check the SYSCTL_DID0_MIN register to determine which to use.
    * Ensure that the main.c setup setup defines the clock frequency as `g_ui32SysClock` for use by this file.
    * Edit the peripheral and GPIO definitions as needed to match the hardware connections being used.
4. Add `extern void SysTickHandler(void);` to *startup_ccs.c in External declarations section. Add `SysTickHandler` to the interrupt vector table on the line labeled `// The SysTick handler`.
