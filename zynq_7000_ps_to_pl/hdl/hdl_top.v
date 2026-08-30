`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:
// Engineer:
//
// Create Date: 08/28/2026 07:50:24 PM
// Design Name:
// Module Name: hdl_top
// Project Name:
// Target Devices:
// Tool Versions:
// Description:
//
// Dependencies:
//
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
//
//////////////////////////////////////////////////////////////////////////////////

module hdl_top (
    // 100 Nhz clock from PL.
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

    // For now send test values to PS.
    assign axi_i_reg_2 = 32'hCAFECAFE;
    assign axi_i_reg_3 = tick_counter;

endmodule
