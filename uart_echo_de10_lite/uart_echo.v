// Clock rate 50 Mhz / 9600 baud.
`define CLKS_PER_BIT_9600 5209
// Clock rate 50 Mhz / 115200 baud.
`define CLKS_PER_BIT_115200 434

module uart_echo (
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
    // Current state of uart receiver data register.
    wire [7:0] uart_rx_byte;
    wire uart_rx_dv;
    wire uart_tx_done;

    // For top-level echo server state machine.
    parameter S_IDLE = 2'b00;
    parameter S_HAS_DATA = 2'b01;
    parameter S_SENDING = 2'b10;
    parameter S_ERROR = 2'b11;

    reg [1:0] echo_state;
    reg uart_tx_start;

    // To send state information to LED[9:8].
    reg [1:0] debug_state;

    always @(negedge clk or negedge rst_n) begin
        if (!rst_n) begin
            echo_state  <= S_IDLE;
            debug_state <= 2'b0;
        end else begin
            // Info to display on LEDs. Previously bitwise or of states reached.
            debug_state <= echo_state;

            case (echo_state)
                S_IDLE: begin
                    if (uart_rx_dv) begin
                        echo_state <= S_HAS_DATA;
                    end
                end

                S_HAS_DATA: begin
                    uart_tx_start <= 1'b1;
                    echo_state <= S_SENDING;
                end

                S_SENDING: begin
                    uart_tx_start <= 1'b0;
                    if (uart_tx_done) begin
                        echo_state <= S_IDLE;
                    end
                end

                default: begin
                    // We should not reach this case; indicate error.
                    echo_state <= S_ERROR;
                end
            endcase
        end
    end

    uart_rx #(
        .CLKS_PER_BIT(`CLKS_PER_BIT_115200)
    ) uart_receiver (
        .i_Clock(clk),
        .i_Rx_Serial(uart_rx),
        .i_Reset(rst_n),
        .o_Rx_DV(uart_rx_dv),
        .o_Rx_Byte(uart_rx_byte),

        // Debug internal state.
        // .o_State_debug(LED[2:0])
    );

    uart_tx #(
        .CLKS_PER_BIT(`CLKS_PER_BIT_115200)
    ) uart_transmitter (
        .i_Clock(clk),
        .i_Tx_DV(uart_tx_start),
        .i_Tx_Byte(uart_rx_byte),
        .i_Reset(rst_n),
        .o_Tx_Active(),
        .o_Tx_Serial(uart_tx),
        .o_Tx_Done(uart_tx_done)
    );

    // LEDs 7 to 0 show start of UART receive byte register.
    assign LED[7:0] = uart_rx_byte;
    // Show state debug info in LEDs 9 and 8.
    assign LED[9:8] = debug_state;

endmodule
