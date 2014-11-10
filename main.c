#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

/********************************************//**
 *  Global Declarations
 ***********************************************/
uint32_t ui32SysClkFreq;


int main(void) {
	char UARTreadChar;
	char idBuffer[7] = {};
	char sentence[20][100] = {};
	bool parsingId = false;
	bool readingData = false;
	uint32_t i = 0; // Sentence id chars
	uint32_t j = 0; // NMEA sentence pointer
	uint32_t k = 0; // NMEA chars

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

	while (1) {
		if (UARTCharsAvail(UART7_BASE)) {
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

					UARTCharPut(UART0_BASE, '\r');
					UARTCharPut(UART0_BASE, '\n');

					/* Debug -- Prints Sentence ID
					while (idBuffer[i] != '\0') {
						UARTCharPut(UART0_BASE, idBuffer[i++]);
					}
					*/
				}
				else {
					readingData = false;
				}
			}
			else if ((parsingId == true) && (UARTreadChar != ',')){
				idBuffer[i] = UARTreadChar;
				i++;
			}
			else if ((readingData == true) && (UARTreadChar != ',')){
				sentence[j][k] = UARTreadChar;
				UARTCharPut(UART0_BASE, sentence[j][k]);
				k++;
			}
			else if ((readingData == true) && (UARTreadChar == ',')){
				sentence[j][k] = '\0';
				UARTCharPut(UART0_BASE, ',');
				j++;
			}
		} // End if chars available
	}
}
