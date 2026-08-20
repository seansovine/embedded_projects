#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

int main() {
    const char *portname = "/dev/ttyUSB0";

    // NOTE: Parameters below suggested by Gemini. vetted bY us.

    // File descriptor parameters:
    //  read/write; cannot be controlling terminal;
    //  writes block until data + metadata are flushed
    int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        std::cerr << "Error opening " << portname << std::endl;
        return 1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "Error from tcgetattr" << std::endl;
        close(fd);
        return 1;
    }

    // Set Baud Rate to 115200.
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

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
    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 5; // 0.5 seconds timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "Error from tcsetattr" << std::endl;
        close(fd);
        return 1;
    }

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

    for (const char ch : msg) {
        // Send one char.
        write(fd, (void *)&ch, 1);
        std::cout << "Sent: " << ch << std::endl;

        // Try to receive one char.
        memset(rxBuf, 0, sizeof(rxBuf));
        int n = read(fd, rxBuf, 1);
        if (n > 0) {
            std::cout << "Received: " << rxBuf[0] << std::endl;
        } else {
            std::cout << "No data received." << std::endl;
        }
    }

    close(fd);
    return 0;
}
