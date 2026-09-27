# Notes on OpenOCD + GDB for STM32

These are brief instructions on how to use OpenOCD and GDB to program an STM32 Nucleo board and run and
debug code on it with the built-in ST-Link interface. The specific board we're using is the STM32F407-DISC1,
but things should work similarly for any board that has an ST-Link programmer.

## Building with ST-Link support

We the cloned OpenOCD source code and we're going to build it with support for the ST-Link programmer.

We needed the following:

```shell
sudo apt install libusb-1.0-0-dev
git clone https://github.com/openocd-org/openocd.git && cd openocd
bear -- ./configure --enable-stlink --prefix=$(realpath ./install) --enable-internal-jimtcl
bear -- make -j 8
make install
```

The version from a distro's package repo is probably good enough for most purposes, but the goal here
was also to study the OpenOCD code itself a bit. This lets us do that and build it with debug symbols.

## Running a server for ST-Link

We first needed to find out the ST-Link version of our board and verify the connection:

```shell
sudo apt install stlink-tools

# Then with board connected to USB:
st-info --probe
```

We Googled the firmware version listed, and found it is ST-Link V2. The `stlink-tools` package also
installs the udev rules to use ST-Link without requiring root priveleges.

We then started an OpenOCD server with:

```shell
install/bin/openocd /
    -f /home/sean/Code_open_source/openocd/install/share/openocd/scripts/interface/stlink.cfg /
    -f /home/sean/Code_open_source/openocd/install/share/openocd/scripts/target/stm32f4x.cfg
```

Note that we used just `stlink.cfg` here. We started with the v2 version, but the console
message said it is deprecated and recommended to just use the `stlink.cfg` version.

For the command output we get:

```shell
> install/bin/openocd [...]
Open On-Chip Debugger 0.12.0+dev-02711-g21f88d781-dirty (2026-09-27-12:47)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
Warn : DEPRECATED: auto-selecting transport "swd (dapdirect)". Use 'transport select swd' to suppress this message.
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : STLINK V2J47M34 (API v2) VID:PID 0483:374B
Info : Target voltage: 2.900353
Info : Unable to match requested speed 2000 kHz, using 1800 kHz
Info : Unable to match requested speed 2000 kHz, using 1800 kHz
Info : clock speed 1800 kHz
Info : SWD DPIDR 0x2ba01477
Info : [stm32f4x.cpu] Cortex-M4 r0p1 processor detected
Info : [stm32f4x.cpu] target has 6 breakpoints, 4 watchpoints
Info : [stm32f4x.cpu] Examination succeed
Info : [stm32f4x.cpu] starting gdb server on 3333
Info : Listening on port 3333 for gdb connections
```

Which looks great and has some interesting things we can learn more about.

## Connecting GDB to flash and debug the board

Starting GDB with the path to our ELF binary for our compiled STM32F407 audio example:

```shell
sudo apt install gdb-multiarch
gdb-multiarch build/f407_audio_example.elf
```

Then within GDB console, with above OpenOCD server command running in a separate window:

```shell
target remote localhost:3333

# Monitor commands are passed to OpenOCD server.
monitor reset halt
# Reload ELF binary onto device.
load
# Tell OpenOcd "to do hard reset, halt at reset vector, run reset-init script".
monitor reset init

# Set breakpoint on main.
b main
# Run to breakpoint.
c
# At breakpoint list surrounding code.
list
```

This shows enough to get started loading, running and debugging code. But there is a lot
more that these tools can do that can be learned about.
