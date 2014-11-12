#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <utils/ustdlib.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"

/********************************************//**
 *  Function Definitions
 ***********************************************/
int printStringToTerminal(char *stringToPrint, int delimiter);
int printFloatToTerminal(float floatToPrint, int delimiter);

/********************************************//**
 *  Global Declarations
 ***********************************************/
uint32_t ui32SysClkFreq;


	int main(void) {

	/********************************************//**
	*  Local Variable Declarations
	***********************************************/
	float	latitude, longitude, speed, course;
	char	*timestamp, *status, *nsIndicator, *ewIndicator, *date;

	char	UARTreadChar;
	char	idBuffer[7] = {};
	char	sentence[15][20] = {};
	bool	parsingId = false;
	bool	readingData = false;

	// UART terminal movement (ANSI/VT100 Terminal Control Escape Sequences)
	// Adopted from the following: http://goo.gl/s43voj
	char 	clearTerminal[] = "\033[2J";
	char	cursorTo_0_0[] = "\033[0;0H";
	char	cursorTo_0_1[] = "\033[2;0H";
	char	header[] = "Time\t\tLatitude\tLongitude\tCourse\t\tSpeed";

	uint32_t	i = 0; // Sentence id chars
	uint32_t	j = 0; // NMEA sentence pointer
	uint32_t	k = 0; // NMEA chars


	/********************************************//**
	*  Port and Clock Configurations
	***********************************************/

	ui32SysClkFreq = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
			SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
			SYSCTL_CFG_VCO_480), 120000000);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PC4_U7RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinConfigure(GPIO_PC5_U7TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1);

	// Debug UART output config
	UARTConfigSetExpClk(UART0_BASE, ui32SysClkFreq, 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	// GPS UART input config
	UARTConfigSetExpClk(UART7_BASE, ui32SysClkFreq, 9600,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));


	/********************************************//**
	*  Main loop
	***********************************************/

	while (1) {
		if (UARTCharsAvail(UART7_BASE)) {
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1, 0);
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
			else if ((parsingId == true) && (UARTreadChar != ',')){
				idBuffer[i] = UARTreadChar;
				i++;
			}
			else if ((readingData == true) && (UARTreadChar == '\r')){
				sentence[j][k] = '\0';

				// Get status of satellite fix. A=Fix. Display error message if none.
				status = strdup(sentence[1]);
				if (strcmp(status, "A") == 0) {
					// Read all the data into correctly typed vars
					// Strings
					timestamp = strdup(sentence[0]);
					nsIndicator = strdup(sentence[3]);
					ewIndicator = strdup(sentence[5]);
					date = strdup(sentence[8]);

					// Floats
					if (strcmp(nsIndicator, "S") == 0) {
						latitude = -1 * ustrtof(sentence[2], NULL);
					}
					else {
						latitude = ustrtof(sentence[2], NULL);
					}

					if (strcmp(ewIndicator, "W") == 0) {
						longitude = -1 * ustrtof(sentence[4], NULL);
					}
					else {
						longitude = ustrtof(sentence[4], NULL);
					}
					// Convert speed from knots to mph
					speed = 1.15078 * ustrtof(sentence[6], NULL);
					course = ustrtof(sentence[7], NULL);

					// Set up cursor/spacing/header
					printStringToTerminal(clearTerminal,0);
					printStringToTerminal(cursorTo_0_0, 0);
					printStringToTerminal(header, 0);
					printStringToTerminal(cursorTo_0_1, 0);

					// Print values to the terminal
					printStringToTerminal(timestamp, 1);
					printFloatToTerminal(latitude, 1);
					printFloatToTerminal(longitude, 1);
					printFloatToTerminal(course, 1);
					printFloatToTerminal(speed, 2);
				}
				else {
					char noFixString[] = "No data available";
					printStringToTerminal(header, 2);
					printStringToTerminal(noFixString, 2);
				}
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
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1, 0xFF);
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
