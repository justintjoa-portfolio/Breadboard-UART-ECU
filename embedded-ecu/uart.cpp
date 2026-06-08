#include "uart_simulator.h"

namespace tiny_ecu {

bool UartSimulator::ReceiveByte(std::uint8_t byte) {
  // TODO: Push byte into rx_buffer_.
  // Return true if accepted, false if full.
}

bool UartSimulator::ReceiveString(const std::string& input) {
  // TODO: Treat input as bytes.
  // Feed each character into ReceiveByte().
  // Return false if any byte cannot be accepted.
}

bool UartSimulator::ReadByte(std::uint8_t* out) {
  // TODO: Return false if out == nullptr.
  // Pop one byte into *out.
  // Return false if empty.
}

bool UartSimulator::ReadLine(std::string* out) {
  // TODO:
  // Return false if out == nullptr.
  // Clear *out.
  // Read bytes until '\n'.
  // Ignore '\r'.
  // Append all other bytes.
  // Return true only if newline was found.
  //
  // Version 1 rule:
  // If no newline exists, partial bytes may be consumed.
}

bool UartSimulator::HasData() const {
  // TODO: Return true if rx_buffer_ is not empty.
}

std::size_t UartSimulator::PendingBytes() const {
  // TODO: Return rx_buffer_ size.
}

}  // namespace tiny_ecu