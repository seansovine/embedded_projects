# Arty Z7 bare metal PS to PL communication

These are practical steps you can use to setup thi

## Create design in Vivado

### Install the board files

This may not be necessary, but if you don't have the files for the board in your Vivado
data repository, the instructions
[here](https://digilent.com/reference/programmable-logic/guides/install-board-files)
explain where to find the Diligent board files and how to install them in the correct
directory where Vivado can find them.

### Create the project

In Vivado, create a new RTL project, selecting "Do not specify sources at this time".

Click Next, and on the next page search for the Arty Z7-20 under the board tab.

We don't actually use board-specific settings in the project, so you could probably just
select the part for the chip that is on the board in this step.

### Create a block design

Click Create Block Design to create a new block design, and in the block design diagram
editor add an instance of the ZYNQ7 Processing System. Then click Run Block Automation on
the banner at the top of the editor to apply the board presets.

Also add a Processor System Reset block and an AXI Interconnect block. It's fine to go ahead
and run Connection Automation now, though we will have to re-run it later. In the interconnect
block make sure that exactly one master and one slave interface is enabled.

### Create a custom AXI IP

In the Tools Menu click Create and Package New IP. On the dialog that opens click Create a
new AXI4 peripheral and click Next. On the next page name your IP -- I used "ps_to_pl_example"
for this example -- and choose where to save it.

On the next page of the dialog, for this tutorial we'll use the default parameters, which
are:

+ Interface type: Lite

+ Interface mode: Slave

+ Data width: 32 bits

+ Number of registers: 4

On the final page go ahead and click "Edit IP", though you can also go back and edit the IP
later from the IP Catalog menu. This will open a new instance of Vivado with the IP project
opened for editing.

__Edit the IP source files:__

In the Hierarchy pane, expand the IP under Design Sources, and open the `_S00_AXI.v` file
for editing. We'll add an output port:

```verilog
    // Users to add ports here
    output wire [3:0] leds,
    // User ports ends
```

Note the definition

```verilog
    reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg0;
```

We will connect our output signal to the low bits of this register, by adding code at the
end of the file:

```verilog
    // Add user logic here
    assign leds = slv_reg0[3:0];
    // User logic ends
```

Add the same input signal to the IP top-level verilog file, and connect it to the input of the
AXI slave module that's instantiated there. For us that is:

```verilog
    // Instantiation of Axi Bus Interface S00_AXI
    ps_to_pl_example_slave_lite_v1_0_S00_AXI # (
        .C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
        .C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
    ) ps_to_pl_example_slave_lite_v1_0_S00_AXI_inst (
        .leds(leds),
        ...
    );
```

__Add the new ports to the IP:__

You should see "Updating..." in the sources pane after these changes are made. In the Package IP
window go to the Ports and Interfaces pane and click in the banner to merge in the changes.

_Note:_ If the option to merge the changes is not visible for some reason, you can go back to the
Verilog file and modify or re-add the new ports and save, and the Ports and Interfaces pane should
update.

__Package the IP:__

In the Package IP window, go to the Review and Package pane and click Re-Package IP. It's fine
to go ahead and close the IP project by clicking Yes.

### Add the IP to the project, connect signals, constrain external ports to package pins

Open the IP Catalog view from the Project Manager pane. Under the User Repository - Axi Peripheral
section you'll see the IP we just created. From here you can open it again for further editing.

Back in the block design Diagram window, click the + icon, locate the new IP and add it.

### Connect the LED signal to the package pins

Right-click the `leds[3:0]` port on the custom AXI IP in the block design and click Make External.

In a minute we will assign package pin constraints to the bits of this signals to connect them to the
LEDs on the board.

### Validate and generate the block design and create HDL wrapper

Click the checkbox icon on the menu bar of the Diagram window. At this point it should say that everything
validated successfully. Then click Generate Block Design. The default options here should work fine.

Now under the Design Sources section of the Sources pane right click the block design and click Create
HDL Wrapper. It's okay to let Vivado manage and auto-update the wrapper. Unless you already created an
additional HDL file, the HDL wrapper should automatically be set as the top module of the project.

### Run synthesis and apply pin constraints

Now in the flow navigator click Run Synthesis. The default settings should work fine.

Once synthesis is complete, go to the Window menu and click I/O Ports. Now in the I/O Ports window
expand the `leds_0` signal and assign an appropriate pin and I/O standard to each bit...
