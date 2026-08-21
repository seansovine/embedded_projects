module led_blinker #(
    parameter CLK_FREQ   = 50_000_000,
    parameter BLINK_RATE = 10
) (
    input  wire clk,     // System clock 1.
    input  wire rst_n,   // Active-low, async reset (pushbutton 0).
    output reg [9:0] LED // LEDR0 - LEDR9.
);

    // 32-bit register to count clock cycles.
    reg  [31:0] clk_counter;

    // 4-bit register to hold curren LED number.
    reg  [ 3:0] current_led;
    // current_led + 1 modulo 10; next LED to illuminate.
    wire [ 3:0] next_led = (current_led + 1'b1) % 10;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            clk_counter <= 32'd0;
            LED         <= 10'b0;
            current_led <= 4'b0;
        end else begin
            // Toggle next illuminated LED every 1 second.
            if (clk_counter >= (CLK_FREQ / BLINK_RATE - 1)) begin
                clk_counter      <= 32'd0;
                LED[current_led] <= 1'b0;
                LED[next_led]    <= 1'b1;
                current_led      <= next_led;
            end else begin
                clk_counter <= clk_counter + 1'b1;
            end
        end
    end

endmodule

module led_blinker_single #(
    parameter CLK_FREQ   = 50_000_000,
    parameter BLINK_RATE = 10
) (
    input  wire clk,     // System clock 1.
    input  wire rst_n,   // Active-low, async reset (pushbutton 0).
    output reg  LED // LEDR0 - LEDR9.
);

    // 32-bit register to count clock cycles.
    reg [31:0] clk_counter;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            clk_counter <= 32'd0;
            LED         <= 10'b0;
        end else begin
            // Toggle next illuminated LED every 1 second.
            if (clk_counter >= (CLK_FREQ / BLINK_RATE - 1)) begin
                clk_counter <= 32'd0;
                LED         <= ~LED;
            end else begin
                clk_counter <= clk_counter + 1'b1;
            end
        end
    end

endmodule
