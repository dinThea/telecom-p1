#ifndef UART_HPP
#define UART_HPP

#include <functional>
#include <deque>
#include <mutex>
#include <vector>
#include <stdint.h>
#include "config.hpp"

enum class State { Waiting, ReceivingStart, Receiving, ReceivingEnd }; 
class UART_RX
{
public:
    UART_RX(std::function<void(uint8_t)> get_byte) :get_byte(get_byte) {
        bits_count = 0;
        sample_count = 0;
        byte = 0;
        byte_counter = 0;
        State state = State::Waiting;
    }
    void put_samples(const unsigned int *buffer, unsigned int n);
    int verify_what_25_out_of_30_last_bits_are();
    void reset_last_bits();
    void add_last_bit(unsigned int bit);

private:
    std::function<void(uint8_t)> get_byte;
    unsigned int bits_count;
    unsigned int sample_count;
    uint8_t byte;
    std::vector<unsigned int> last_bits;
    unsigned int byte_counter;
    State state;

};

class UART_TX
{
public:
    void put_byte(uint8_t byte);
    void get_samples(unsigned int *buffer, unsigned int n);
private:
    std::deque<unsigned int> samples;
    std::mutex samples_mutex;
    void put_bit(unsigned int bit);
};

#endif
