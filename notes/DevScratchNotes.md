# Developer Scratch Notes

## DE10 Lite UART project

We now have this done.

We used a UART receiver module found on [Nand Land](https://nandland.com/uart-serial-port-module/),
and modified it slightly to add an asynchronous reset and the ability to debug its internal state.
We also had add more bits to its clock tick register to accomodate a faster clock with a slower
baud rate. We added the UART transmitter module from that same page, and a top-level state machine
so that the device receives a byte an immediately sends it back, then waits for the next byte.

We can also expore using a faster data rate. We used a slower rate to rule that out as a problem,
but it could be that the insufficient tick counter width was causing the problem with the faster
rate too.

## Future work: Higher-bandwidth communication protocols

The UART project is really a test run, and lets us set up a system to help debug our designs.
But for actual use of the FPGA we'll want to be able to communicate between the FPGA and the controller
PC or MCU board with a much higher data rate.

For this we can try out different protocols and interfaces that are available for our two current FPGA
dev boards. For the DE10 Lite, we could try SPI, which would require an internal clock domain crossing.
Our Arty Z7 board has a much wider range of physical interfaces and AMD / Xilinx IP to make use of them.
