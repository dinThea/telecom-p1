#include "uart.hpp"
#include <iostream>
#include <bitset>

void UART_RX::put_samples(const unsigned int *buffer, unsigned int n)
{

    for (int i = 0; i < n; i++) {
        if (state == State::Waiting) {
            if (buffer[i] != 1)
            {
                state = State::ReceivingStart;
                sample_count++;
            }
        } else if (state == State::ReceivingStart) {
            if (sample_count < (SAMPLES_PER_SYMBOL / 2 + (SAMPLES_PER_SYMBOL * 3) / 32))
            {
                sample_count++;
            }
            else
            {
                int last_bit = verify_what_25_out_of_30_last_bits_are();
                reset_last_bits();
                if (last_bit == -1 | last_bit == 1)
                {
                    state = State::Waiting;
                    sample_count = 0;
                    continue;
                }
                else
                {
                    state = State::Receiving;
                    sample_count = 0;
                    continue;
                }
            }
        } else if (state == State::Receiving) {
            if (bits_count < 7) {
                if (sample_count < SAMPLES_PER_SYMBOL-1) {
                    sample_count++;
                }
                else
                {
                    sample_count = 0;
                    int last_bit = verify_what_25_out_of_30_last_bits_are();
                    reset_last_bits();
                    if (last_bit == -1)
                    {
                        state = State::Waiting;
                        sample_count = 0;
                        continue;
                    }
                    byte |= (last_bit << bits_count);
                    bits_count++;
                }
            } else {
                if (sample_count < SAMPLES_PER_SYMBOL - 1)
                {
                    sample_count++;
                }
                else
                {
                    int last_bit = verify_what_25_out_of_30_last_bits_are();
                    reset_last_bits();
                    if (last_bit == -1)
                    {
                        state = State::Waiting;
                        sample_count = 0;
                        continue;
                    }
                    byte |= (last_bit << bits_count);
                    byte_counter++;
                    std::bitset<8> bit_byte(byte);
                    get_byte(byte);
                    byte = 0;
                    sample_count = 2;
                    bits_count = 0;
                    state = State::ReceivingEnd;
                }
            }
        } else if (state == State::ReceivingEnd) {
            if (sample_count < (SAMPLES_PER_SYMBOL / 2))
            {
                sample_count++;
            }
            else
            {
                int last_bit = verify_what_25_out_of_30_last_bits_are();
                reset_last_bits();
                sample_count=0;
                state = State::Waiting;
            }
        }
        add_last_bit(buffer[i]);
    }
}

void UART_RX::reset_last_bits()
{
    last_bits.clear();
}

void UART_RX::add_last_bit(unsigned int bit)
{
    last_bits.push_back(bit);
}

int UART_RX::verify_what_25_out_of_30_last_bits_are()
{
    // Verifies if 25 out of the last 30 bits are 1 or 0 (returns -1 if there are less than 25 bits)
    int zero_count = 0;
    int one_count = 0;
    if (last_bits.size() < 30)
    {
        return -1;
    }
    for (int i = 0; i < last_bits.size(); i++)
    {
    }

    for (int i = last_bits.size() - 30 - 1; i < last_bits.size(); i++)
    {
        if (last_bits[i] == 1)
        {
            one_count++;
        }
        else
        {
            zero_count++;
        }
    }
    if (one_count > 25)
    {
        return 1;
    }
    else if (zero_count > 25)
    {
        return 0;
    }
    return -1;
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
