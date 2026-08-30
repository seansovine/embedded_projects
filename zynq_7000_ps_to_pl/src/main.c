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
  xil_printf(" - Register 2 after write: %08X\n\r", reg_2_val);
  reg_3_val = Xil_In32(AXI_REG_3);
  xil_printf(" - Register 3 after write: %08X\n\r", reg_3_val);

  for (u32 i = 0; i < 16; ++i) {
    xil_printf("\n\rIteration %02u:\n\r", i);

    // Not necessary:
    // Xil_DCacheInvalidateRange(AXI_REG_BASEADDR, 32);

    reg_3_val = Xil_In32(AXI_REG_3);
    xil_printf(" - Tick count register (reg 3): %010u\n\r", reg_3_val);

    float seconds = reg_3_val / 100000000.0;
    printf(" - In seconds: %.8f\n\r", seconds);

    usleep(250 * 1000);
  }

  print("\nDone!\n\r");
  cleanup_platform();
  return 0;
}