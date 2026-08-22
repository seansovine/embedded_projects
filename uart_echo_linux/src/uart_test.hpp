#ifndef UART_TEST_HPP_
#define UART_TEST_HPP_

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// ------------------------
// TTY UART test functions.

/*
 * Sends and receives data according to the following client/server protocol:
 *
 * The "server" on te board receives on byte at a time and then immediately echos
 * it back. We send then receive here based on that expectation.
 */
inline void echo_test_repeat(int file_descriptor, int64_t byteDelay = 250) {
    static const std::string TEST_MSG           = "Hello UART!";
    static constexpr uint32_t NUM_TRANSMISSIONS = 100;

    for (uint32_t trans = 0; trans < NUM_TRANSMISSIONS; ++trans) {
        std::cout << "\n>> Sending message " << trans << ":" << std::endl;

        for (const char ch : TEST_MSG) {
            // Send one char.
            write(file_descriptor, (void *)&ch, 1);
            std::cout << "Sent: " << ch << std::endl;

            // Try to receive one char.
            char rxBuf = 0;
            int n      = read(file_descriptor, &rxBuf, 1);

            if (n > 0) {
                std::cout << "Received: " << rxBuf << std::endl;
            } else {
                std::cout << "No data received." << std::endl;
            }

            // Delay between bytes sent/received.
            std::this_thread::sleep_for(std::chrono::milliseconds(byteDelay));
        }

        // Delay between complete message transmissions.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

/*
 * Sends a sequence of bytes with short delay between. Does not attempt receive.
 */
inline void send_single_bytes(int file_descriptor, uint32_t bytesToSend = 8) {
    static constexpr std::array<uint8_t, 8> SEND_VALUES = {0xFF, 0x00, 0x0F, 0xF0, 0xAA, 0x55, 0xCC, 0x33};

    for (uint32_t send_i = 0; send_i < bytesToSend; ++send_i) {
        uint8_t sendData = SEND_VALUES[send_i % std::size(SEND_VALUES)];

        std::cout << "Sending byte: 0x" << std::hex << std::setw(2) << std::setfill('0') << (unsigned)sendData
                  << std::endl;
        write(file_descriptor, (void *)&sendData, 1);
        std::cout << " - Send complete." << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

#endif // UART_TEST_HPP_
