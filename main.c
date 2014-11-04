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

uint32_t ui32SysClkFreq;

void UARTIntHandler(void) {
	uint32_t ui32Status;
	const char delimiter[2] = ",$";
	bool gpmrcDone = false;
	char sentence[508] = {};
	char sentenceToParse[508] = {};
	char parsedOutput[508] = {};
	uint8_t i = 0;
	char UARTreadChar[1] = {};
	char *str, *delim, *currentParsed;

	ui32Status = UARTIntStatus(UART7_BASE, true); //get interrupt status
	UARTIntClear(UART7_BASE, ui32Status); //clear the asserted interrupts

	//loop while there are chars
	while(UARTCharsAvail(UART7_BASE)) {
		//DEBUG UARTreadChar = UARTCharGetNonBlocking(UART7_BASE);
		//DEBUG strcat(buildChar, UARTreadChar);
		sprintf(UARTreadChar, "%c", UARTCharGetNonBlocking(UART7_BASE));
		strcat(sentence, UARTreadChar);
		// LEDs on
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1, 0xFF);
	}

	delim = strdup(delimiter);
	str = strdup(sentence);

    currentParsed = strtok(sentenceToParse, "$");
    while (gpmrcDone != true) {
        currentParsed = strtok(NULL, "$");
        currentParsed = strtok(NULL, ",");

        if (strcmp(currentParsed,"GPRMC") == 0) {
            for (i = 0; i < 12; i++) {
                currentParsed = strtok(NULL, delimiter);
				strcat(parsedOutput, currentParsed);
            }
            gpmrcDone = true;
        }
    }

	//echo character to debug term
	for (i = 0; i < strlen(parsedOutput); i++) {
		//UARTCharPutNonBlocking(UART0_BASE, sentence[i]);
		UARTCharPut(UART0_BASE, parsedOutput[i]);
	}
	// LEDs off
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1, 0x00);
}


int main(void) {
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

	IntMasterEnable();
	IntEnable(INT_UART7);
	UARTIntEnable(UART7_BASE, UART_INT_RX | UART_INT_RT);

	while (1) {
	}
}
