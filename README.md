# Embedded development projects

This will contain various projects for learning and practicing embedded development
concepts on microcontrollers, FGPAs and CPUs.

## DE10 Lite LED blink

This is a basic project example for developing in Quartus (21.1 is used here) for the
Terasic DE10 Lite dev board, which has an Altera Max 10 FPGA chip on it. This contains
just the three files needed to open and build the project in Quartus and program and
run it on the device.

## UART communication

This is a simple UART echo server example. It has a Linux PC as client sending messages
over a ttyUSB device to a USB-to-UART bridge to an echo server implemented on a
microcontroller or FPGA development board. It prints the data sent to and echoed back
from the server to verify the server is working correctly.

Relevant files are in:

+ [uart_echo_linux](./uart_echo_linux/)

+ [uart_echo_stm32f407](./uart_echo_stm32f407/)

+ [uart_echo_fpga](./uart_echo_fpga/)
