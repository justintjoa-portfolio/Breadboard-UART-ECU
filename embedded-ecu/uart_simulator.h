#ifndef UART_SIMULATOR_H
#define UART_SIMULATOR_H

#include "ring_buffer.h" 


namespace tiny_ecu {

class UartSimulator {
    public: 
        UartSimulator();
        // Delete copy constructor. 
        UartSimulator(const UartSimulator&) = delete; 
        // Delete copy assignment. 
        UartSimulator& operator=(const UartSimulator&) = delete; 
        // Remove move constructor. 
        UartSimulator(UartSimulator&&) = delete; 
        // Remove move assignment. 
        UartSimulator& operator=(UartSimulator&&) = delete; 

        bool ReceiveByte(std::uint8_t byte);
        bool ReceiveString(const std::string& input);

        bool ReadByte(std::uint8_t* out);
        bool ReadLine(std::string* out);

        bool HasData() const;
        std::size_t PendingBytes() const;

    private: 
        RingBuffer<uint8_t, 256> rx_buffer_;
};



};
#endif // UART_SIMULATOR_H
