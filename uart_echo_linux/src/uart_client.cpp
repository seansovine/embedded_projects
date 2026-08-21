#include "uart_lib.hpp"

#include <cstdint>
#include <cstring>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

enum class Mode : uint32_t {
    EchoRepeat,
    SendOnce,
};

static constexpr Mode MODE = Mode::SendOnce;

// 9_600 for FPGA test ; 115_200 for microcontroller test.
static constexpr const speed_t BAUD_RATE = B9600;
static constexpr const char *PORT_NAME   = "/dev/ttyUSB0";

int main() {
    std::optional<TtyPort> port = TtyPort::create(PORT_NAME, BAUD_RATE);
    if (!port) {
        return 1;
    }

    switch (MODE) {
        case Mode::EchoRepeat: {
            echo_test_repeat(port->fd());
            break;
        }
        case Mode::SendOnce: {
            send_single_bytes(port->fd());
            break;
        }
    }

    port->close_fd();
    return 0;
}
