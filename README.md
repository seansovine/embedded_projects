# Embedded development projects

This will contain various projects for learning and practicing embedded development
concepts on microcontrollers, FGPAs and CPUs.

## UART communication

This is a simple UART echo server example. It has a Linux PC as client sending messages
over a ttyUSB device to a USB-to-UART bridge to an echo server implemented on a
microcontroller or FPGA development board. It prints the data sent to and echoed back
from the server to verify the server is working correctly.

Relevant files are in:

+ [uart_echo_linux](./uart_echo_linux/)

+ [uart_echo_stm32f407](./uart_echo_stm32f407/)

+ [uart_echo_fpga](./uart_echo_fpga/)
