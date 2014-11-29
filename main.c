#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <utils/ustdlib.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/fpu.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"

//**********************************
//! Function Definitions
 //**********************************
int printStringToTerminal(char *stringToPrint, int delimiter);
int printFloatToTerminal(float floatToPrint, int delimiter);
int logToSD(char *inTimestamp, char *inDate, float inLatitude, float inLongitude, float inSpeed, float inCourse);
float convertCoordinate(float inCoordinate, const char *direction);
int chipDetect(void);
int gpsData(void);

//*****************************************************************************
//! Global Definitions
//*****************************************************************************

//*****************************************************************************
//
//! The following are data structures used by FatFs.
//
//*****************************************************************************
static FATFS g_sFatFs;
static FIL g_sFileObject;

//*****************************************************************************
//
//! The system clock frequency in Hz.
//
//*****************************************************************************
uint32_t g_ui32SysClock;

//*****************************************************************************
//
//! The update rate and current pulse count
//! Update rate = updateRate+1
//
//*****************************************************************************
uint32_t updateRate = 0;
uint32_t updateCounter = 0;

//*****************************************************************************
//
//! This is the Pulse Per Second interrupt handler.
//! The updateCounter is incremented on each Pulse per second call, if equal to
//! the update rate, the GPS data is parsed and logged.
//
//*****************************************************************************
void PortKIntHandler(void) {
	uint32_t intStatus = 0;

	//
	// Get the current interrupt status for Port K
	//
	intStatus = GPIOIntStatus(GPIO_PORTK_BASE,true);

	//
	// Clear the set interrupts for Port K
	//
	GPIOIntClear(GPIO_PORTK_BASE,intStatus);

	//
	// Execute code for PK2 interrupt
	//
	if((intStatus & GPIO_INT_PIN_2) == GPIO_INT_PIN_2){
		if (updateRate == updateCounter++) {
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x02);
			gpsData();
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00);
			updateCounter = 0;
		}
	}
}


int main(void) {

	//*************************************************************************
	//! I/O config and setup
	//*************************************************************************

	g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
			SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
			SYSCTL_CFG_VCO_480), 120000000);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);	// UART
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);	// UART
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);	// UART0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);	// UART7
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);	// SSI
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);	// GPIO
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);		// SSI
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);	// GPIO

	// UART0 and UART7
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PC4_U7RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinConfigure(GPIO_PC5_U7TX);

	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	// LED indicators
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// SD Chip Detect (PK3) and GPS Pulse Per Second (PK2)
	GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_2|GPIO_PIN_3);
	// Pulse Per Second input pin config as weak pull-down
	GPIOPadConfigSet(GPIO_PORTK_BASE,GPIO_PIN_2,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPD);
	// Pulse Per Second input pin config as rising edge triggered interrupt
	GPIOIntTypeSet(GPIO_PORTK_BASE,GPIO_PIN_2,GPIO_RISING_EDGE);
	// Register Port K as interrupt
	GPIOIntRegister(GPIO_PORTK_BASE, PortKIntHandler);
	// Enable Port K pin 2 interrupt
	GPIOIntEnable(GPIO_PORTK_BASE, GPIO_INT_PIN_2);

	GPIOPinConfigure(GPIO_PD0_SSI2XDAT1);
	GPIOPinConfigure(GPIO_PD1_SSI2XDAT0);
	GPIOPinConfigure(GPIO_PD2_SSI2FSS);
	GPIOPinConfigure(GPIO_PD3_SSI2CLK);

	// SD Chip Detect (CD) - weak pull-up input
	GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	// Debug UART output config
	UARTConfigSetExpClk(UART0_BASE, g_ui32SysClock, 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	// GPS UART input config
	UARTConfigSetExpClk(UART7_BASE, g_ui32SysClock, 9600,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	//
	// Configure SysTick for a 100Hz interrupt.
	//
	SysTickPeriodSet(g_ui32SysClock / 100);
	SysTickIntEnable();
	SysTickEnable();

	//
	// Floating point enable
	//
	FPUEnable();
	FPULazyStackingEnable();

	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x00);

	//
	// While forever
	//
	while (1) {
	}
}


//*****************************************************************************
//
//! Prints the string passed to the terminal
//!
//! \param stringToPrint points to the string to print.
//! \param delimiter denotes delimiter type. 0=none, 1=tab, 2=newline
//
//*****************************************************************************
int printStringToTerminal(char *stringToPrint, int delimiter) {
	uint8_t i = 0;
	while (stringToPrint[i] != '\0') {
		UARTCharPut(UART0_BASE, stringToPrint[i++]);
	}

	if (delimiter == 1) {
		UARTCharPut(UART0_BASE, '\t');
	}
	else if (delimiter == 2) {
		UARTCharPut(UART0_BASE, '\r');
		UARTCharPut(UART0_BASE, '\n');
	}

	return 0;
} // End function printStringToTerminal


//*****************************************************************************
//
//! Prints the number passed to the terminal
//!
//! \param floatToPrint points to the string to print.
//! \param delimiter denotes delimiter type. 0=none, 1=tab, 2=newline
//
//*****************************************************************************
int printFloatToTerminal(float floatToPrint, int delimiter) {
	uint8_t i = 0;
	char stringToPrint[80];

	sprintf(stringToPrint, "%f", floatToPrint);
	while (stringToPrint[i] != '\0') {
		UARTCharPut(UART0_BASE, stringToPrint[i++]);
	}

	if (delimiter == 1) {
		UARTCharPut(UART0_BASE, '\t');
	}
	else if (delimiter == 2) {
		UARTCharPut(UART0_BASE, '\r');
		UARTCharPut(UART0_BASE, '\n');
	}

	return 0;
} // End function printFloatToTerminal


//*****************************************************************************
//
//! This logs the data to the SD card
//!
//! \param inTimestamp as a string
//! \param inDate as a string
//! \param inLatitude as float
//! \param inLongitude as float
//! \param inSpeed as float
//! \param inCourse as float
//
//*****************************************************************************

int logToSD(char *inTimestamp, char *inDate, float inLatitude,
			float inLongitude, float inSpeed, float inCourse) {

	FRESULT iFResult;   // File function return code
	UINT 	bw;         // amount of bytes written to SD
	char data[6][30] = {};
	char logOutputString[100] = {};
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t k = 0;

	strcpy(data[0], inTimestamp);
	strcpy(data[1], inDate);
	sprintf(data[2], "%f", inLatitude);
	sprintf(data[3], "%f", inLongitude);
	sprintf(data[4], "%f", inSpeed);
	sprintf(data[5], "%f", inCourse);

	//GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0xFF);

	//
	// Create a tab delimited string to write to SD card.
	//
	for (i = 0; i < 6; i++) {
	    j = 0;
		while (data[i][j] != '\0') {
			logOutputString[k++] = data[i][j++];
		}
		if (i != 5) {
			logOutputString[k++] = '\t';
		}
		else {
			logOutputString[k] = '\n';
			logOutputString[k + 1] = '\0';
		}
	}

    //
    // Mount the file system, using logical disk 0 and write to SD card
    //
	iFResult = f_mount(0, &g_sFatFs);
    if (iFResult != FR_OK) {
    	// TODO: add output to error_log.txt file
    }
    else {
    	//
    	// Open/create file and append data to the end.
    	//
    	f_open(&g_sFileObject, "/gps_log.txt", FA_WRITE | FA_OPEN_ALWAYS);
    	f_lseek(&g_sFileObject, f_size(&g_sFileObject));
    	f_write(&g_sFileObject, logOutputString, ustrlen(logOutputString), &bw);
    	f_close(&g_sFileObject);
		if (bw == ustrlen(logOutputString)) {
			//
			// TODO: add an indicator for successful SD write
			//
		}
    }

    //
    // Unmount disk
    //
    f_mount(0, NULL);

    return 0;
}

//*****************************************************************************
//
//! Returns a GPS coordinate in decimal format.
//!
//! \param inCoordinate is latitude or longitude as a float.
//! \param direction is the N,S,E,W indicator in as a string.
//
//*****************************************************************************
float convertCoordinate(float inCoordinate, const char *direction) {
    float dd, mm, outputCoordinate;

    dd = floor(inCoordinate * .01);
    mm = (inCoordinate - (dd*100)) / 60;
    outputCoordinate = dd + mm;

    if (strcmp(direction, "S") == 0) {
        return outputCoordinate *= -1;
    }
    else if (strcmp(direction, "W") == 0) {
        return outputCoordinate *= -1;
    }
    else {
        return outputCoordinate;
    }
}

//*****************************************************************************
//
//! This is the handler for this SysTick interrupt. FatFs requires a timer tick
//! every 10 ms for internal timing purposes.
//
//*****************************************************************************
void SysTickHandler(void) {
    //
    // Call the FatFs tick timer.
    //
    disk_timerproc();
}

//*****************************************************************************
//
//! This reads the Chip Detect pin of the SD card and returns the state.
//! Function returns 1 if card is present, returns 0 if not.
//
//*****************************************************************************
int chipDetect(void) {
	volatile uint32_t chipDetectStatus = 0; // SD Chip Detect Pin

	chipDetectStatus = GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_3);
	return chipDetectStatus;
}

//*****************************************************************************
//
//! This reads the data from the GPS module, parses the data, and saves to
//! the microSD card, if a card is present.
//
//*****************************************************************************
int gpsData(void) {
    float	latitude, longitude, speed, course;
    char	UARTreadChar;
    char	idBuffer[7] = {};
    char	sentence[10][20] = {};
    bool	parsingId = false;
    bool	readingData = false;
    bool    readComplete = false;

    uint32_t	i = 0; 	// Sentence id chars
    uint32_t	j = 0; 	// NMEA sentence pointer
    uint32_t	k = 0; 	// NMEA chars

    while (readComplete != true) {
    	//GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x00);
    	if (UARTCharsAvail(UART7_BASE)) {
    		//GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0xFF);
    		UARTreadChar = UARTCharGet(UART7_BASE);

    		if ((parsingId == false) && (UARTreadChar == '$')) {
    			i = 0;
    			parsingId = true;
    			readingData = false;
    		}
    		else if ((parsingId == true) && (UARTreadChar == ',')) {
    			idBuffer[i] = '\0';
    			i = 0;
    			parsingId = false;

    			if (strcmp(idBuffer, "GPRMC") == 0) {
    				j = 0;
    				k = 0;
    				readingData = true;
    			}
    			else {
    				readingData = false;
    			}
    		}
    		else if ((parsingId == true) && (UARTreadChar != ',')) {
    			idBuffer[i] = UARTreadChar;
    			i++;
    		}
    		else if ((readingData == true) && (UARTreadChar == '\r')) {
    			sentence[j][k] = '\0';

    			//
    			// Copy data which will remain strings into named variables
    			//
    			char *timestamp = strdup(sentence[0]);
    			char *status = strdup(sentence[1]);
    			char *nsIndicator = strdup(sentence[3]);
    			char *ewIndicator = strdup(sentence[5]);
    			char *date = strdup(sentence[8]);

    			//
    			// Process latitude
    			//
    			latitude = convertCoordinate(ustrtof(sentence[2], NULL), nsIndicator);

    			//
    			// Process longitude
    			//
    			longitude = convertCoordinate(ustrtof(sentence[4], NULL), ewIndicator);

    			//
    			// Convert speed from knots to mph
    			//
    			speed = 1.15078 * ustrtof(sentence[6], NULL);
    			course = ustrtof(sentence[7], NULL);

    			//****************************************************
    			//! Data Debug/Display UART terminal movement
    			//!
    			//! (ANSI/VT100 Terminal Control Escape Sequences)
    			//! Adapted from the following: http://goo.gl/s43voj
    			//****************************************************
///*DEBUG ->
    			// Initial terminal setup
    			// Clear Terminal
    			printStringToTerminal("\033[2J",0);
    			// Cursor to 0,0
    			printStringToTerminal("\033[0;0H", 0);
    			printStringToTerminal("Time (UTC)", 0);
    			//Cursor to 0,1
    			printStringToTerminal("\033[2;0H", 0);
    			// Print values to the terminal
    			printStringToTerminal(timestamp, 2);
    			printStringToTerminal("\r\n", 0);
    			printStringToTerminal("Latitude\tLongitude", 2);
    			printFloatToTerminal(latitude, 1);
    			printFloatToTerminal(longitude, 2);
    			printStringToTerminal("\r\n", 0);
    			printStringToTerminal("Course\t\tSpeed (MPH)", 2);
    			printFloatToTerminal(course, 1);
    			printFloatToTerminal(speed, 2);
    			if (chipDetect()) {
    				printStringToTerminal("\r\n*Logging to SD Card*", 2);
    			}
//<- DEBUG*/

    			//
    			// Check if SD card is present and write data if true.
    			//
    			if (chipDetect()) {
    				logToSD(timestamp, date, latitude, longitude, speed, course);
    			}

    			// Deallocate memory used by strdup function
    			free(timestamp);
    			free(status);
    			free(nsIndicator);
    			free(ewIndicator);
    			free(date);

    			return 0;
    		}
    		else if ((readingData == true) && (UARTreadChar != ',')){
    			sentence[j][k] = UARTreadChar;
    			k++;
    		}
    		else if ((readingData == true) && (UARTreadChar == ',')){
    			sentence[j][k] = '\0';
    			j++;
    			k = 0;
    		}
    	} // End if chars available
    } // end while

    return -1;
}
