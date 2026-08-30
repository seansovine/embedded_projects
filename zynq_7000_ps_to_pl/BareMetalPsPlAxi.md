# Arty Z7 bare metal PS to PL communication

This tutorial contains practical steps you can use to setup an example project in Vivaod + Vitis for
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
editor , right-click the `leds[3:0]` port on the custom HDL IP in the block design and click
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

The C source for the example program we will create is in [`main.c`](src/main.c).
The USB JTAG interface on the board has a UART-to-USB serial bridge built in, which you can
connect to using a serial console, to read from stdout of the processing system code. I use the Tio serial
console for this on Linux.

_TODO:_ Add full steps for this and mention potential issues and tools for debugging.

> _Next installment coming soon._
