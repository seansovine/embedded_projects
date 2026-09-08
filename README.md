# Embedded development projects

This contains various projects for learning and practicing embedded development concepts
on microcontrollers, FGPAs and CPUs. It will serve to consolidate some of my knowledge and
will provide templates for quickly getting started with similar projects.

It will also serve as a meta-project linking to other projects of mine on related topics;
see [External projects](#external-projects) below.

And, [this](#interesting-work-by-others) section contains some interesting work by others
that I've tried out and found to be useful or interesting.

## Arty Z7 Zynq 7000 projects

These are tutorials and example projects for the Digilent
[Arty Z7](https://digilent.com/shop/arty-z7-zynq-7000-soc-development-board/)
board, which has an AMD/Xilinx Zynq 7000 SoC FPGA chip on it. They will explain in detail
the process of setting up projects using the Vivado + Vitis development workflow to use various
features of the hardware and supporting software.

### Bare metal PS to PL communication: [tutorial example](zynq_7000_ps_to_pl/BareMetalPsPlAxi.md)

This shows how to create a design in Vivado with a custom AXI IP that you can use to send and
receive data between the processing system (PS) running a bare metal program and the programmable logic
(PL) running a design created from Verilog sources. It includes bare metal C code using the Xilinx
embedded SDK to send and receive data to and from AXI shared registers and Verilog source code for a PL
design that writes to these registers and uses values read from them to control the board LEDs.
This version uses AXI-Lite with the PS as master.

<p align="center" margin="20px">
        <img src="https://github.com/seansovine/page_images/blob/main/photos/Arty%20Z7-20%20and%20laptop%20-%2020260829.jpg?raw=true"
        alt="image of MCU echo server connected to PC" width="600" style="padding-top: 10px; padding-bottom: 10px"/>
</p>

### Interesting Zynq resources

Here are a few interesting resources I've found on the Zynq 7000 architecture and PS-to-PL communication:

- [The Zynq Book](https://www.zynqbook.com/) by the University of Strathclyde

- This 2020 Purdue [thesis](https://hammer.purdue.edu/articles/EFFICIENT_IMPLEMENTATION_OF_SOBEL_EDGE_DETECTION_WITH_ZYNQ-7000/12252704/files/22543922.pdf)
  on the efficiency of Zynq 7000 communication

## UART communication

This is a simple UART echo server example. It has a Linux PC as client sending messages
over a ttyUSB device to a USB-to-UART bridge to an echo server implemented on a microcontroller
or FPGA development board. It prints the data sent to and echoed back from the server to verify
the server is working correctly. It can be used as a template for setting up a UART to
control or debug a microcontroller or FPGA.

**The relevant subprojects are:**

### Linux C++ application: [uart_echo_linux](./uart_echo_linux/)

This is a simple C++ program and library with a class that manages a ttyUSB device connection
that is configured to communicate with a UART over a USB-to-UART bridge, and a couple functions
for testing sending and receiving data over such a connection.

The bridge device I use is [this one](https://www.amazon.com/dp/B07WX2DSVB), which has the
popular FTDI chip in it. It has a jumper for choosing the logic level on the board it interfaces,
though all my boards are currently using 3.3V.

_Testing the MCU version in my home workspace:_

<p align="center" margin="20px">
	<img src="https://github.com/seansovine/page_images/blob/main/photos/STM32F407%20UART%20echo%20-%2020260820_164504.jpg?raw=true"
        alt="image of MCU echo server connected to PC" width="800" style="padding-top: 10px; padding-bottom: 10px"/>
</p>

### STM32 dev board C application: [uart_echo_stm32f407](./uart_echo_stm32f407/)

This is a simple UART echo server implemented on the STM32F407-DISC1 development board from
ST Microelectronics. For ST development I am now using the CubeMX generated CMake project +
VS Code STM32 extension pack workflow. I am very happy with this environment for development.

In particular I like that CubeMX generates a portable CMake project with only the necessary initialization
code and HAL and libary files included, and it provides code sections in the generated
main source files, marked with

```C
  /* USER CODE BEGIN * */

  ...

  /* USER CODE END * */
```

that it won't touch if you update the CubeMX configuration and regenerate the project. If you add
additional files in the `Core/` directory, it won't touch those either. The VS Code extension pack
also has solid integration with CMake, Clangd and GDB.

This microcontroller version of the server can be used to validate that things are setup correctly
on the Linux PC side.

### DE10 Lite FPGA design: [uart_echo_de10_lite](./uart_echo_de10_lite/)

This implements the simple UART echo server on the Terasic DE10 Lite dev board, which has an Altera
Max 10 FPGA chip on it. It includes the Verilog source files and the necessary project files to open
and build the design in Quartus.

I got the UART receiver and transmitter modules from [Nand Land](https://nandland.com/uart-serial-port-module/)
and modified them very slightly to add an asynchronous reset (because we're generating a reset from a
pushbutton for this design) and to allow debugging the internal state while I was initially
getting the design working correctly.

The main Verilog file for the design is: [`uart_echo.v`](uart_echo_de10_lite/uart_echo.v). It is
setup so that the rightmost 8 LEDs on the board show the bits of the internal byte buffer of the
UART receiver, and the leftmost 2 LEDs show the current state of the top-level state machine. Since
the server is idle during most clock cycles, the state LEDs are usually turned off, but you can see
a PWM effect as it zips through the send states.

_DE10 Lite board running the echo server:_

<p align="center" margin="20px">
	<img src="https://github.com/seansovine/page_images/blob/main/photos/DE10%20Lite%20UART%20echo%20close%20-%2020260822_105944.jpg?raw=true"
        alt="image of DE10 Lite board running echo server" width="550" style="padding-top: 10px; padding-bottom: 10px"/>
</p>

## DE10 Lite LED blink

This is a basic project example for developing in Quartus (21.1 is used here) for the Terasic
DE10 Lite dev board, which has an Altera Max 10 FPGA chip on it. It blinks the 10 LEDs on the
board in sequence, once per second. The project folder contains just the files needed to open
and build the project in Quartus and program and run it on the device. This can serve as a quick
template for Quartus projects for the DE10 Lite board.

## External projects

Here are some other projects I have posted publicly on topics related to embedded development:

### Rust embedded development for STM32F407-DISC1: [stm32_rust](https://github.com/seansovine/stm32_rust)

This has some basic examples showing how to get started and demonstrating some of the tools
available in the embedded Rust ecosystem. It is mostly drawn from other projects and tutorials
and adapted to this particular dev board, with comments on practical steps to get things working.

### STM32F407-DISC1 accelerometer input device: [stm32f407_accelerometer](https://github.com/seansovine/stm32f407_accelerometer)

This contains STM32 development board code and a Rust client library to use the on-board accelerometer
as an input device on Linux.

### STM32F407-DISC1 TinyUSB integration: [stm32f407_usb](https://github.com/seansovine/stm32f407_usb)

Example integrating the TinyUSB embedded USB library into an STM32F4 CMake project generated by CubeMX,
allowing development in the ST VS Code environment. Configures the full-speed USB peripheral as a
communication class device and as an audio input device.

### BeaglePlay single-board computer projects: [beagleplay_projects](https://github.com/seansovine/beagleplay_projects)

Projects for developing on the BeaglePlay single-board Arm Linux computer, with a focus on sensor
interfacing and embedded Linux.

<p align="center" margin="20px">
	<img src="https://github.com/seansovine/page_images/blob/main/photos/BeaglePlay%20BMP280%20-%2020260804_074528.jpg?raw=true"
        alt="image of DE10 Lite board running echo server" height="800" style="padding-top: 10px; padding-bottom: 10px"/>
</p>

### Linux camera streaming and conversion: [linux_camera](https://github.com/seansovine/linux_camera)

This is an example of using the FFmpeg libavdevice and libavcodec APIs to stream frames from an
H264 USB camera and convert them to JPEG images. It's not strictly an embedded project, but I am
running it on the BeaglePlay single board computer.

## Interesting work by others

### RISCV CPU for DE10 Lite board: [RISCV](https://github.com/ShaheerSajid/RISCV) by Saheer Sajid

I've built this project in Quartus 21.1 and followed his instructions to compile and run code with
the toolchain he links to, using my UART-to-USB bridge and a Linux serial console app (I like tio)
as the stdout console. It is very cool to have a completely open CPU project that is so easy to
setup and run.
