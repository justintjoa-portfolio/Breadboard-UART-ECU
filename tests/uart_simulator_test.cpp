#include "embedded-ecu/uart_simulator.h"

#include <cassert>
#include <cstdint>
#include <string>

int main() {
  tiny_ecu::UartSimulator uart;

  assert(!uart.HasData());
  assert(uart.PendingBytes() == 0);

  std::uint8_t byte = 0;
  assert(!uart.ReadByte(&byte));
  assert(!uart.ReadByte(nullptr));

  assert(uart.ReceiveByte(static_cast<std::uint8_t>('A')));
  assert(uart.HasData());
  assert(uart.PendingBytes() == 1);

  assert(uart.ReadByte(&byte));
  assert(byte == static_cast<std::uint8_t>('A'));
  assert(!uart.HasData());
  assert(uart.PendingBytes() == 0);

  assert(uart.ReceiveString("OK\n"));

  std::string line;
  assert(uart.ReadLine(&line));
  assert(line == "OK");
  assert(!uart.HasData());

  assert(uart.ReceiveString("SET SPEED 55\r\n"));
  assert(uart.ReadLine(&line));
  assert(line == "SET SPEED 55");

  assert(!uart.ReadLine(nullptr));

  return 0;
}