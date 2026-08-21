#ifndef UART_LIB_HPP_
#define UART_LIB_HPP_

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// -------------------------------------------
// Class representing Linux TTY serial device.

class TtyPort {
public:
    TtyPort(int file_descriptor, struct termios tty)
        : file_descriptor_(file_descriptor),
          tty_(tty) {
    }

    static std::optional<TtyPort> create(const std::string &portName, speed_t baudRate) {
        // File descriptor parameters:
        //  read/write; cannot be controlling terminal;
        //  writes block until data + metadata are flushed
        int fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) {
            std::cerr << "Error opening " << portName << std::endl;
            return std::nullopt;
            ;
        }

        struct termios tty;
        if (tcgetattr(fd, &tty) != 0) {
            std::cerr << "Error from tcgetattr" << std::endl;
            close(fd);
            return std::nullopt;
            ;
        }

        // Set baud rate.
        cfsetispeed(&tty, baudRate);
        cfsetospeed(&tty, baudRate);

        // Control modes math configuration on board end:
        //  8 bits, no parity, 1 stop bit;
        //  disable hardware flow control.
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit characters
        tty.c_cflag &= ~PARENB;                     // No parity
        tty.c_cflag &= ~CSTOPB;                     // 1 stop bit
        tty.c_cflag &= ~CRTSCTS;                    // No flow control
        tty.c_cflag |= CREAD | CLOCAL;              // Turn on READ & ignore ctrl lines

        // Local modes:
        //  Raw input (disable canonical mode, echo, signals)
        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        // Input modes:
        //  No software flow control. (TODO: Read up on it.)
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);

        // Output modes:
        //  Raw output; no post-processing.
        tty.c_oflag &= ~OPOST;

        // Read settings:
        //  block until at least 1 byte arrives, w/ 0.5 s timeout.
        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 5; // 0.5 seconds timeout

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            std::cerr << "Error from tcsetattr" << std::endl;
            close(fd);
            return std::nullopt;
            ;
        }

        return TtyPort{fd, tty};
    }

    int fd() {
        return file_descriptor_;
    }

    void close_fd() {
        assert(file_descriptor_ >= 0);
        close(file_descriptor_);
    }

private:
    int file_descriptor_ = -1;
    struct termios tty_;
};

// ---------------
// Test functions.

inline void echo_test_repeat(int file_descriptor) {
    // Test string and receive buffer (only using 1 byte).
    std::string msg = "Hello UART!";
    char rxBuf[2];

    /*
     * Note on client/server protocol:
     *
     *  The "server" on te board receives on byte at a time
     *  and then immediately echos it back. We send then
     *  receive here based on that expectation.
     */

    static constexpr uint32_t NUM_TRANS = 100;

    for (uint32_t trans = 0; trans < NUM_TRANS; ++trans) {
        std::cout << "\n>> Sending message " << trans << ":" << std::endl;

        for (const char ch : msg) {
            // Send one char.
            write(file_descriptor, (void *)&ch, 1);
            std::cout << "Sent: " << ch << std::endl;

            // Try to receive one char.
            memset(rxBuf, 0, sizeof(rxBuf));
            int n = read(file_descriptor, rxBuf, 1);
            if (n > 0) {
                std::cout << "Received: " << rxBuf[0] << std::endl;
            } else {
                std::cout << "No data received." << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

inline void send_single_bytes(int file_descriptor) {
    static constexpr std::array<uint8_t, 8> SEND_VALUES = {0xFF, 0x00, 0x0F, 0xF0, 0xAA, 0x55, 0xCC, 0x33};

    for (uint32_t send_i = 0; send_i < std::size(SEND_VALUES); ++send_i) {
        uint8_t sendData = SEND_VALUES[send_i];

        std::cout << "Sending byte: 0x" << std::hex << std::setw(2) << std::setfill('0') << (unsigned)sendData
                  << std::endl;
        write(file_descriptor, (void *)&sendData, 1);
        std::cout << " - Send complete." << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

#endif // UART_LIB_HPP_
