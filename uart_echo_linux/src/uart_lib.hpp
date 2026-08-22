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

// -------------------------------------------
// Class representing Linux TTY serial device.

class TtyPort {
public:
    static std::shared_ptr<TtyPort> create(const std::string &portName, speed_t baudRate) {
        // File descriptor parameters:
        //   read/write; cannot be controlling terminal;
        //   writes block until data + metadata are flushed
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

        // Set baud rate.
        cfsetispeed(&tty, baudRate);
        cfsetospeed(&tty, baudRate);

        // Control modes math configuration on board end:
        //   8 bits, no parity, 1 stop bit;
        //   disable hardware flow control.
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit characters
        tty.c_cflag &= ~PARENB;                     // No parity
        tty.c_cflag &= ~CSTOPB;                     // 1 stop bit
        tty.c_cflag &= ~CRTSCTS;                    // No flow control
        tty.c_cflag |= CREAD | CLOCAL;              // Turn on READ & ignore ctrl lines
        // Local modes:
        //   Raw input (disable canonical mode, echo, signals)
        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        // Input modes:
        //   No software flow control. (TODO: Read up on it.)
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        // Output modes:
        //   Raw output; no post-processing.
        tty.c_oflag &= ~OPOST;
        // Read settings:
        //   block until at least 1 byte arrives, w/ 0.5 s timeout.
        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 5; // 0.5 seconds timeout

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            std::cerr << "Error from tcsetattr" << std::endl;
            close(fd);
            return nullptr;
        }

        return std::make_shared<TtyPort>(fd, tty);
    }

    int fd() {
        return file_descriptor_;
    }

    bool initialized() {
        return file_descriptor_ >= 0;
    }

    TtyPort(int file_descriptor, struct termios tty)
        : file_descriptor_(file_descriptor),
          tty_(tty) {
    }

    ~TtyPort() {
        close_fd();
    }

private:
    void close_fd() {
        assert(file_descriptor_ >= 0);
        close(file_descriptor_);
    }

    int file_descriptor_ = -1;
    struct termios tty_;
};

#endif // UART_LIB_HPP_
