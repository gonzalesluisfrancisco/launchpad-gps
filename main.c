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

/********************************************//**
 *  Global Declarations
 ***********************************************/
uint32_t ui32SysClkFreq;


int main(void) {
	char UARTreadChar;
	char idBuffer[7] = {};
	char sentence[400] = {};
	bool parsingId = false;
	uint32_t i = 0;
	uint32_t j = 0;

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

	UARTConfigSetExpClk(UART0_BASE, ui32SysClkFreq, 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	// GPS UART config
	UARTConfigSetExpClk(UART7_BASE, ui32SysClkFreq, 9600,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	while (1) {
		if (UARTCharsAvail(UART7_BASE)) {
			UARTreadChar = UARTCharGet(UART7_BASE);

			if ((parsingId == false) && (UARTreadChar == '$')) {
					i = 0;
					parsingId = true;
					i++;
			}
			else if ((parsingId == true) && (UARTreadChar == ',')) {
					idBuffer[i] = NULL;
					parsingId = false;
					i++;
					UARTCharPut(UART0_BASE, '\r');
					UARTCharPut(UART0_BASE, '\n');
			}
			else if ((parsingId == true) && (UARTreadChar != ',')){
				idBuffer[i] = UARTreadChar;
			}

			sentence[i] = UARTreadChar;
			if (strlen(idBuffer) <= 6) {
				UARTCharPut(UART0_BASE, idBuffer[i]);
			}
		}
	}
}
