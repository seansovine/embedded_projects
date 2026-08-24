#ifndef UART_LIB_HPP_
#define UART_LIB_HPP_

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// --------------------------------------------
// Linux TTY serial device configured for uart.

class TtyPort {
public:
    static std::shared_ptr<TtyPort> create(const std::string &portName, speed_t baudRate) {
        // File descriptor parameters:
        //   - read/write;
        //   - cannot be controlling terminal;
        //   - writes block until data + metadata are flushed
        int fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

        if (fd < 0) {
            std::cerr << "Error opening " << portName << std::endl;
            return nullptr;
        }

        struct termios tty;

        if (tcgetattr(fd, &tty) != 0) {
            std::cerr << "Error from tcgetattr" << std::endl;
            close(fd);
            return nullptr;
        }

        cfsetispeed(&tty, baudRate);
        cfsetospeed(&tty, baudRate);

        // Configure UART message to match config on the boards:
        //   - 8 data bits
        //   - no parity bit
        //   - 1 stop bit
        //   - disable hardware flow control.
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;
        tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl line.

        // Local modes:
        //   - Raw input (disable canonical mode, echo, signals)
        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        // Input modes:
        //   - No software flow control. (TODO: Read up on it.)
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);

        // Output modes:
        //   - Raw output; no post-processing.
        tty.c_oflag &= ~OPOST;

        // Read settings:
        //   - 0.5 second timeout whether or not data was received.
        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 5;

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            std::cerr << "Error from tcsetattr" << std::endl;
            close(fd);
            return nullptr;
        }

        return std::shared_ptr<TtyPort>(new TtyPort(fd, tty));
    }

    int fd() {
        assert(initialized());
        return file_descriptor_;
    }

    bool initialized() {
        return file_descriptor_ >= 0;
    }

    ~TtyPort() {
        close_fd();
    }

private:
    /// Takes ownership of the file descriptor; closes it on destruction.
    TtyPort(int file_descriptor, struct termios tty)
        : file_descriptor_(file_descriptor) {
    }

    void close_fd() {
        assert(initialized());
        close(file_descriptor_);
    }

    int file_descriptor_ = -1;
};

#endif // UART_LIB_HPP_
