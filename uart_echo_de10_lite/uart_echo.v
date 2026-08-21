module uart_echo #(
    parameter CLK_FREQ   = 50_000_000,
    parameter BLINK_RATE = 10
) (
    // System clock 1.
    input wire clk,
    // Active-low, async reset (pushbutton 0).
    input wire rst_n,
    // UART receive signal.
    input wire uart_rx,

    // LEDR9 - LEDR0.
    output wire [9:0] LED,
    // UART transmit signal.
    output wire uart_tx
);
    // Blink LED to show design is running.
    led_blinker_single blinker (
        .clk  (clk),
        .rst_n(rst_n),
        .LED  (LED[9])
    );

    // Current state of uart receiver data register.
    wire [7:0] uart_rx_byte;
    wire uart_rx_dv;
    reg uart_received;

    always @(posedge clk or negedge rst_n or posedge uart_rx_dv) begin
        if (!rst_n) begin
            uart_received <= 1'b0;
        end else begin
            uart_received <= uart_received | uart_rx_dv;
        end
    end

    uart_rx #(
        // Set to CLK_FREQ / 115_200.
        .CLKS_PER_BIT(434)
    ) uart_receiver (
        .i_Clock(clk),
        .i_Rx_Serial(uart_rx),
        .o_Rx_DV(uart_rx_dv),
        .o_Rx_Byte(uart_rx_byte)
    );

    // LEDs 7 to 0 show start of UART receive byte register.
    assign LED[7:0] = uart_rx_byte;
    assign LED[8]   = uart_received;

endmodule
