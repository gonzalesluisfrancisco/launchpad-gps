#Tiva C Connected Launchpad GPS Data Logger

## UART

1. Begin with UART interrupt version of lab10 as a basic template.
2. To allow for easy debugging, use two UARTs. One UART for GPS input and another that echoes the input to a terminal.

    a. Since UART0_BASE is already configured as a PC serial connection, use UART1_BASE for GPS input.

    b. Use Port C pins 4 and 5 as GPS UART input.


Pin Location  | Port and pin | Function
------------- | -------------|----------
J1-3          | PC4          | RX
J1-4          | PC5          | TX
