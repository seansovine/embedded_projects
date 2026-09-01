# Arty Z7 bare metal PS to PL communication

This tutorial contains practical steps you can use to setup an example project in Vivado + Vitis for
the Arty Z7 (or other Zynq 7000) board that includes:

+ A custom AXI IP with four 32-bit registers shared between the PS and PL

+ A Verilog HDL design for the PL that reads from two of these and writes to two of them

+ A bare metal C program using the Xilinx embedded SDK that reads and writes from these registers

### Sources:

The information here has been gathered from various AMD/Xilinx documents and from online forums
and videos. Hopefully this contains everything that is needed in one place, with warnings and
workarounds for potential stumbling blocks one might run into.
There a few similar tutorials available online, including
[this one](https://www.hackster.io/j-abate/integrating-zynq-ps-and-pl-with-memory-mapped-registers-292a42),
which gave me the idea to separate the AXI registers into a set writable by the PL and a set
writable by the PS.

## Create a design in Vivado

### Install the board files

This may not be necessary, but if you don't have the files for the board in your Vivado
data repository, the instructions
[here](https://digilent.com/reference/programmable-logic/guides/install-board-files)
explain where to find the Diligent board files and how to install them in the correct
directory where Vivado can find them.

### Create the project

In Vivado, create a new RTL project, selecting "Do not specify sources at this time".
Click Next, and on the next page search for the Arty Z7-20 under the board tab.
Note that we don't actually use board-specific settings in the project, so you can probably
just select the chip that is on the board in this step on the part tab, instead of choosing
the board.

### Create a block design

Click Create Block Design in the Flow Navigator to create a new block design, and in the block design
Diagram window add an instance of the ZYNQ7 Processing System. Then click Run Block Automation on
the banner at the top of the editor to apply the board presets.

You can add now a Processor System Reset block and an AXI Interconnect block now, but I find
it's easier to let Connection Automation add them for you in a later step. It will connect things
correctly, and that will rule out one more source of error if you run the design on the board and
it doesn't behave as expected.

### Create a custom AXI IP

In the Tools Menu click Create and Package New IP. In the dialog that opens click Create a
new AXI4 peripheral and click Next. On the next page name your IP and choose where to save it.
The name you choose here will also show up in constants that are generated in the supporting
embedded C code.

On the next page of the dialog we'll use the default parameters, which
are:

+ Interface type: Lite

+ Interface mode: Slave

+ Data width: 32 bits

+ Number of registers: 4

On the final page we'll go ahead and click Edit IP, but you can also go back and edit the IP
later from the IP Catalog menu. This will open a new instance of Vivado with the IP project
opened for editing.

By default the IP you created should have two slave devices, one called `S00_AXI` and one called
`S_AXI_INTR`. We won't use the interrupt here but we will keep it in the design, because doing so
avoids a known issue in some versions of Vivado that causes a single AXI slave to be optimized in such
a way that it becomes unresponsive to reads. This issue is documented in the Vivado forums.

__Edit the IP source files:__

We will update the example Verilog files generated for the IP. The versions with our modifications are in:

+ [`baremetal_ps_to_pl_v2.v`](hdl/axi_custom_ip/baremetal_ps_to_pl_v2.v)

+ [`baremetal_ps_to_pl_v2_slave_lite_v1_0_S00_AXI.v`](hdl/axi_custom_ip/baremetal_ps_to_pl_v2_slave_lite_v1_0_S00_AXI.v)

+ [`baremetal_ps_to_pl_v2_slave_lite_inter_v1_0_S_AXI_INTR.v`](hdl/axi_custom_ip/baremetal_ps_to_pl_v2_slave_lite_inter_v1_0_S_AXI_INTR.v)

(Side note, we use the [Verible](https://github.com/chipsalliance/verible) tool to format our Verilog files.)

We will make modifications so that:

+ registers 0 and 1 are only written to by the PS

+ registers 2 and 3 are only written to by the PL

In the Hierarchy pane, expand the IP under Design Sources, and open the `*_S00_AXI.v` file
for editing. We'll add couple input and output ports to allow other logic in the PL to access
the registers:

```verilog
    // Users to add ports here
    output wire [C_S00_AXI_DATA_WIDTH-1:0] o_reg_0,
    output wire [C_S00_AXI_DATA_WIDTH-1:0] o_reg_1,
    input  wire [C_S00_AXI_DATA_WIDTH-1:0] i_reg_2,
    input  wire [C_S00_AXI_DATA_WIDTH-1:0] i_reg_3,
    // User ports ends
```

Note the definitions of these registers the generated source file:

```verilog
    reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg0;
    reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg1;
    reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg2;
    reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg3;
```

We will expose the values in registers 0 and 1 to the PL:

```verilog
	// Add user logic here
    assign o_reg_0 = slv_reg0;
    assign o_reg_1 = slv_reg1;
	// User logic ends
```

And, we will modify the AXI write state machine so that it doesn't touch registers
2 and 3 (lines 226-254):

```verilog
            2'h2:
            for (
                byte_index = 0;
                byte_index <= (C_S_AXI_DATA_WIDTH / 8) - 1;
                byte_index = byte_index + 1
            )
            if (S_AXI_WSTRB[byte_index] == 1) begin
                // Disallowing master writes to reg 2.
            end
            2'h3:
            for (
                byte_index = 0;
                byte_index <= (C_S_AXI_DATA_WIDTH / 8) - 1;
                byte_index = byte_index + 1
            )
            if (S_AXI_WSTRB[byte_index] == 1) begin
                // Disallowing master writes to reg 3.
            end
            default: begin
                // This case statement now only assigns to regs 0 and 1.
                slv_reg0 <= slv_reg0;
                slv_reg1 <= slv_reg1;
            end
        endcase
    end

    // Update output regs on every clock rising edge.
    slv_reg2 <= i_reg_2;
    slv_reg3 <= i_reg_3;
```

Then we add the same new ports to the IP top-level Verilog file, `baremetal_ps_to_pl_v2.v`,
and connect them to the ports of the AXI slave module that is instantiated there. For us that is:

```verilog
    // Instantiation of Axi Bus Interface S00_AXI
    baremetal_ps_to_pl_v2_slave_lite_v1_0_S00_AXI #(
        .C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
        .C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
    ) baremetal_ps_to_pl_v2_slave_lite_v1_0_S00_AXI_inst (
        .o_reg_0(o_reg_0),
        .o_reg_1(o_reg_1),
        .i_reg_2(i_reg_2),
        .i_reg_3(i_reg_3),
        ...
        // Generated signal connections.
    );
```

__Add the new ports to the IP:__

You should see "Updating..." appear at the top of in the Sources pane after these changes are made. In the
Package IP window go to the Ports and Interfaces pane and click the text that should appear in the banner
there to merge the changes from the HDL files into the table.

_Note:_ If the option to merge the changes is not visible for some reason, you can go back to the
Verilog file and modify or delete and re-add the new ports and save, and the link to merge in
changes from the HDL should then appear in the banner of the Ports and Interfaces pane.

__Package the IP:__

In the Package IP window, go to the Review and Package pane, which is the last option in the list,
and click Re-Package IP. You can choose the option in the popup window to close the package IP project.

### Add the IP to the project, connect signals and regenerate block design

Open the IP Catalog view from the Project Manager pane. Under the User Repository -> Axi Peripheral
section you'll see the IP we just created. From here you can open it again for further editing.

Back in the block design Diagram window, click the + icon, locate the new IP in the search window,
and add it to the block design. If you already have an instance of this IP in your design, the block
design editor in the main Vivado window will prompt you to upgrade your IP and regenerate the block
design output files.

### Validate and generate the block design and create HDL wrapper

Click the checkbox icon on the menu bar of the Diagram window to validate the block design. At this point
it should say that everything validated successfully. Then click Generate Block Design. The default options
here should work fine.

Now under the Design Sources section of the Sources pane right click the block design and click Create
HDL Wrapper. It's okay to let Vivado manage and auto-update the wrapper. Unless you already created an
additional HDL file, the HDL wrapper should automatically be set as the top module of the project.

_Note:_ It's important that your block design is the top module for the project. If you create another
HDL file before the block design wrapper, that file will be set as top by default. This will cause
validation errors in later build steps.

### Add a custom HDL IP block

In the Sources pane, click the + sign and use the dialog that appears to create a new Verilog source
file and add it to the project. You can create HDL code that uses the signals from the AXI IP registers
in whatever way you'd like. Here we have created the following simple module to test out all the
capabilities:

```verilog
module hdl_top (
    // 100 Mhz clock from PL.
    input clock,

    // Outputs from AXI IP registers.
    input [31:0] axi_o_reg_0,
    input [31:0] axi_o_reg_1,

    // Inputs to AXI IP registers.
    output [31:0] axi_i_reg_2,
    output [31:0] axi_i_reg_3,

    // To four regular LEDs on board.
    output [3:0] leds
);
    // Keep 32-bit count of clock ticks w/ wraparound.
    reg [31:0] tick_counter;

    always @(posedge clock) begin
        tick_counter <= tick_counter + 1;
    end

    // Assign low bits of AXI reg 0 to LEDs, if enabled.
    assign leds = (axi_o_reg_1 == 32'b0) ? axi_o_reg_0[3:0] : 32'b0;

    // Send test value to this register.
    assign axi_i_reg_2 = 32'hCAFECAFE;
    // Send counter value to be stored by the AXI IP.
    assign axi_i_reg_3 = tick_counter;

endmodule
```

This is in file [`hdl_top.v`](hdl/hdl_top.v).

__Create IP block from Verilog source:__

Now drag this file from the Sources pane and drop it in the Diagram window. This will create a new
IP block from this source. Then make the appropriate connections between the inputs and
outputs of the custom AXI IP and this module, and connect this module's clock port to the `FCLK_CLK0`
of the ZYNQ7 Processing System block, so that this module and the AXI IP use a common clock.

Now it's a good idea to validate the design again. Once it validates that everything is connected
correctly, you can click Generate Block Design in the Flow Navigator to generate the output files
for use in the next build steps.

### Connect the LED signal to the package pins

Now we will assign our design's external ports to physical pins on the chip package. In the Diagram
editor, right-click the `leds[3:0]` port on the custom HDL IP in the block design and click
Make External. In the next few steps we will assign package pin constraints to the individual bits
of this signals to connect them to the LEDs on the board and assign appropriate IO standards.

### Run synthesis and apply pin constraints

Now in the flow navigator click Run Synthesis. The default settings should work fine. When synthesis
is complete, open the synthesized design.

Now go to the Window menu and click I/O Ports to open the I/O ports assignment editor. In that editor,
expand the `leds_0` signal and assign an appropriate pin and I/O standard to each individual bit.
The file [`constraints.xdc`](design/constraints.xdc) has the appropriate pin constraints for the Art Z7-20
board. The full set of constraints for this board are available from Digilent online.

### Run implementation, generate bitstream and export hardware

Once the constraints have been added, in the Flow Navigator click Run Implementation. The default settings
are fine. Once implementation is complete, open the implemented design and click Generate Bitstream.

_Note:_ In some versions of Vivado you need to have the implemented design open to generate the bitstream,
or you will see an error message logged to the console that indicates that it may not have run correctly.

Finally, once the bitstream is generated, go to Export Hardware under the File menu. In the Export Hardware
window choose Include Bitstream and choose a location to save the generated `.xsa` file to. We will import
this file in Vitis in a later step to create a hardware platform.

## Create a bare metal platform and test application in Vitis

The C source for the example program we will create is in [`main.c`](src/main.c). We are using the free
2024.1 version of the Vitis Unified IDE that is bundled with the same version of Vivado.

### Getting output from the board

The USB JTAG interface on the board has a UART-to-USB serial bridge built in, which you can
connect to using a serial console, to read from stdout of the processing system code. I use the Tio serial
console for this on Linux. A glance at `dmesg` after connecting the board's programmer USB to you PC and
powering it up will show you which USB devices were created for the board.

The programmer USB actually creates two serial devices: In my setup they are `/dev/ttyUSB0` and
`/dev/ttyUSB1`. The second one is configured for use by the `printf` statements in the running embedded
code, so that's the one I connect my serial console to.

### Create an embedded platform using exported hardware file

Open Vitis and either from the welcome page or from File menu -> New Component open the dialog to create
a new platform component. In the Create Platform Component dialog name the component and choose a workspace
folder location. Vitis uses a single folder as its workspace and automatically detects and components
exist in immediate subfolders of that workspace.

On the next page choose the Hardware Design option and use Browse to find and select the `.xsa` file that
we exported after implementing our design in Vivado. Then on the next page we will keep "standalone" for
the operating system, because we want this project to run on bare metal with no OS, and you can choose
either processor to target. Since there is no OS, to use both processors we'd have to program them
separately and manage shared resources and communication between them.

__Configure and build the platform:__

You will see the newly-created platform component in the Vitis Components pane in the upper left part of
the Vitis workspace.  If you expand its item, then expand Settings, then double-clicking `vitis-comp.json`
will open the platform configuration for editing. In particular, the "Switch XSA" option on the top-level
page there allows you to rebuild the platform from a new `.xsa` file. This lets you make updates to the
platform without recreating any application components you've created that depend on it.

In the Flow pane under Vitis Components, select the platform component if it isn't already and click Build.
In the Output pan at the bottom of the screen you will see log output that should end with "Platform Build
Finished successfully".

### Create a bare metal application

You can create an application from scratch, but I find it much easier to start with one of the examples,
so that's what we'll do here. Go to the Welcome page or to File -> New Component, and choose the option
to create a new component from examples. We'll choose the "hello world" example, as a a good starting place
for a bare metal C application to run on our chosen CPU core.

After choosing the example you can change the name, then we will choose the platform we defined in this
workspace from the exported `.xsa` hardware description, and we will choose the one domain in our platform.
According to the docs a domain is a combination of a process and a board support package (BSP) or an OS.
A platform can have multiple domains that run simultaneously.

__Potential config fixes:__

Sometimes some of the launch configuration information gets misconfigured when you first
create an application. I'm not sure what sequence of actions triggers these, but I'm in the habit of fixing
these up front to avoid doing it later. If you expand Settings under the application in the Vitis Components
pane and open `launch.json`, you'll see text boxes for Bitstream File and Initialization File.

I use Browse to point the first of these to the `.bit` file in the directory of the Vivado project used to generate
the `.xsa` file. This makes sure that if you regenerate the Vivado project the newly-generated bitstream file will
used to configure the PL. Sometime this won't update on its own with the initial configuration.

Then, we want to make sure that the Initialization File input points to the `ps7_init.tcl` file in the `_ide/psinit`
subfolder. Sometimes it gets pointed at a TCL script for initializing the IP, and the application project
will then fail to run.

### Modifying the main C source file

To cut right to the chase, I modified the generated `helloworld.c` file to the following:

```c
#include "platform.h"
#include "xparameters.h"

#include "sleep.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_printf.h"

#include <stdio.h>

static const u32 AXI_REG_BASEADDR = XPAR_BAREMETAL_PS_TO_PL_V2_0_BASEADDR;
static const u32 AXI_REG_0 = AXI_REG_BASEADDR + 0x00;
static const u32 AXI_REG_1 = AXI_REG_BASEADDR + 0x04;
static const u32 AXI_REG_2 = AXI_REG_BASEADDR + 0x08;
static const u32 AXI_REG_3 = AXI_REG_BASEADDR + 0x0C;

static const float PL_CLK_HZ = 100000000.0;

int main() {
  init_platform();

  print("AXI PS-to_PL:\n\r");
  Xil_Out32(AXI_REG_0, 0x0000000Cu);
  Xil_Out32(AXI_REG_1, 0x00000000u);

  u32 reg_0_val = 0;
  u32 reg_1_val = 0;
  u32 reg_2_val = 0;
  u32 reg_3_val = 0;

  reg_0_val = Xil_In32(AXI_REG_0);
  xil_printf(" - Register 0 after write: %08X\n\r", reg_0_val);
  reg_1_val = Xil_In32(AXI_REG_1);
  xil_printf(" - Register 1 after write: %08X\n\r", reg_1_val);
  reg_2_val = Xil_In32(AXI_REG_2);
  xil_printf(" - Register 2 initial: %08X\n\r", reg_2_val);
  reg_3_val = Xil_In32(AXI_REG_3);
  xil_printf(" - Register 3 initial: %08X\n\r", reg_3_val);

  for (u32 i = 0; i < 16; ++i) {
    xil_printf("\n\rIteration %02u:\n\r", i);
    Xil_Out32(AXI_REG_0, i);

    reg_3_val = Xil_In32(AXI_REG_3);
    xil_printf(" - Tick count register (reg 3): %010u\n\r", reg_3_val);

    // Should be ~1/4 second from last reading.
    float seconds = reg_3_val / PL_CLK_HZ;
    printf(" - In seconds: %.8f\n\r", seconds);

    // Delay 1/4 second.
    usleep(250 * 1000);
  }

  print("\nDone!\n\r");
  cleanup_platform();
  return 0;
}

```

If you add `#include "xparameters.h"`, you can use Clangd to see that this generated header file lives in
the platform source directories. The base address in its definition can also be seen in the Address window
of the block diagram view in Vivado. It is the memory address that the PS MMU maps to the AXI shared registers
that were generated for our custom IP. Since they were each 32 bits and are mapped sequentially, we can access
each next one by adding four to the base address, which we do in the following definitions.

After this, the code is straightforward: We write to the two registers we setup for writing from the PS, and
we read all four values. In particular, remember that the value of register 3 was a count of the PL clock
that was updated in the PL on every tick. And in Zynq 7000 the clock that is output from the PS to the PL
is derived from the same reference clock that drives the PS, divided to the appropriate frequency by a PLL
in the PS. So looking at this clock tick can give some very rough sense of the latency and variance of getting
data to and from the PL.

Note also the definition (simplified slightly):

```c
static INLINE void Xil_Out32(UINTPTR Addr, u32 Value)
{
	/* write 32 bit value to specified address */
	volatile u32 *LocalAddr = (volatile u32 *)Addr;
	*LocalAddr = Value;
}
```

This makes our assignment pass through a volatile pointer. This keeps the compiler from making optimizations
based on the assumption that the value stored at `Addr` only changes through code it sees. Since this points
to mapped memory shared with the PL that assumption would be invalid, potentially leading to the code having
stale values if the PL updates the variable and the compiler doesn't generate code that reloads it on each read.

### Debugging and further discussion

I plan to add more to this section in the future. But for now, I'll mention that Vitis has excellent integration
with the Target Communication Framework (TCF) debugger. And, Vivado has the Integrated Logic Analyzer (ILA)
that can be used to monitor and capture the values of signals in the PL as it's running. I have seen this put
to good use by posters in the Xilinx forums for debugging issues with the AXI communication state. I tend to
use printing to the console and even the board LEDs to help track the state of the PS and PL at a high level
as I'm developing. But there are many tools I haven't learned yet and taken advantage of.
