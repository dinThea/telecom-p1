#include "uart.hpp"
#include <iostream>
#include <bitset>

void UART_RX::put_samples(const unsigned int *buffer, unsigned int n)
{

    for (int i = 0; i < n; i++) {
        if (state == State::Waiting) {
            if (buffer[i] == 1) {
                continue;
            } else {
                // std::cout << std::endl;
                // std::cout << "Receiving Start" << std::endl;
                // std::cout << buffer[i];
                state = State::ReceivingStart;
                sample_count++;
            }
        } else if (state == State::ReceivingStart) {
            if (sample_count < SAMPLES_PER_SYMBOL) {
                // std::cout << buffer[i];
                sample_count++;
            } else {
                // std::cout << std::endl;
                // std::cout << "Receiving" << std::endl;
                // std::cout << buffer[i];
                sample_count=0;
                state = State::Receiving;
            }
        } else if (state == State::Receiving) {
            if (bits_count < 7) {
                if (sample_count < SAMPLES_PER_SYMBOL-1) {
                    sample_count++;
                    // std::cout << buffer[i];
                } else {
                    // std::cout << std::endl;
                    sample_count = 0;
                    // std::cout << "BIT: "<< buffer[i-1] << std::endl;
                    // std::cout << buffer[i];
                    byte |= (buffer[i-1] << bits_count);
                    // byte <<= 1;
                    bits_count++;
                }
            } else {
                byte |= (buffer[i-1] << bits_count);
                byte_counter ++;
                std::bitset<8> bit_byte(byte);
                // std::cout << "BYTE NO " << byte_counter << " " << bit_byte << std::endl;
                get_byte(byte);
                byte = 0;
                // std::cout << "Receiving End" << std::endl;
                // std::cout << buffer[i];
                sample_count=2;
                bits_count=0;
                state = State::ReceivingEnd;
            }
        } else if (state == State::ReceivingEnd) {
            if (sample_count < SAMPLES_PER_SYMBOL - 1) {
                sample_count++;
                // std::cout << buffer[i];
            } else {
                // std::cout << buffer[i] << std::endl;
                // std::cout << "Waiting" << std::endl;
                sample_count=0;
                state = State::Waiting;
            }
        }
    }
}

void UART_TX::put_byte(uint8_t byte)
{
    samples_mutex.lock();
    put_bit(0);  // start bit
    for (int i = 0; i < 8; i++) {
        put_bit(byte & 1);
        byte >>= 1;
    }
    put_bit(1);  // stop bit
    samples_mutex.unlock();
}

void UART_TX::get_samples(unsigned int *buffer, unsigned int n)
{
    samples_mutex.lock();
    std::vector<unsigned int>::size_type i = 0;
    while (!samples.empty() && i < n) {
        buffer[i++] = samples.front();
        samples.pop_front();
    }
    samples_mutex.unlock();

    while (i < n) {
        // idle
        buffer[i++] = 1;
    }
}

void UART_TX::put_bit(unsigned int bit)
{
    for (int i = 0; i < SAMPLES_PER_SYMBOL; i++) {
        samples.push_back(bit);
    }
}
