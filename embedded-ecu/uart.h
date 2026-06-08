#ifndef TINY_ECU_UART_H_
#define TINY_ECU_UART_H_

#include <cstddef>
#include <cstdint>
#include <string>

#include "ring_buffer.h"

namespace tiny_ecu {

class UartSimulator {
 public:
  static constexpr std::size_t kRxBufferSize = 256;

  UartSimulator() = default;

  bool ReceiveByte(std::uint8_t byte);
  bool ReceiveString(const std::string& input);

  bool ReadByte(std::uint8_t* out);
  bool ReadLine(std::string* out);

  bool HasData() const;
  std::size_t PendingBytes() const;

 private:
  RingBuffer<std::uint8_t, kRxBufferSize> rx_buffer_;
};

}  // namespace tiny_ecu

#endif  // TINY_ECU_UART_H_