module uart_echo #(
      parameter CLK_FREQ   = 50_000_000,
      parameter BLINK_RATE = 10
) (
      input wire clk,   // System clock 1.
      input wire rst_n, // Active-low, async reset (pushbutton 0).

      output wire [9:0] LED  // LEDR0 - LEDR9.
);

    // Blink LEDs to show design is running.
    led_blinker blinker (
          .clk  (clk),
          .rst_n(rst_n),
          .LED  (LED)
    );

    // TODO: Find UART Verilog IP and use it to build echo server.

endmodule
