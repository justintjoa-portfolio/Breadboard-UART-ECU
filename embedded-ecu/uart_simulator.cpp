#include "uart_simulator.h"


namespace tiny_ecu {

    bool UartSimulator::ReceiveByte(std::uint8_t byte) {
        return rx_buffer_.Push(byte);
    }

    bool UartSimulator::ReceiveString(const std::string& input) {
        bool successful = false; 
        while (rx_buffer_.Pop())

    }

    bool UartSimulator::ReadByte(std::uint8_t* out) {
        return rx_buffer_.Pop(out);
    }

    bool UartSimulator::ReadLine(std::string* out) {
        rx_buffer_.Size();
    }

    bool UartSimulator::HasData() const {
        return ! rx_buffer_.Empty();
    }

    std::size_t UartSimulator::PendingBytes() const {
        return rx_buffer_.Size();
    }


};
