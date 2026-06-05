#include "tiny_ecu/ring_buffer.h"

#include <cassert>
#include <cstdint>

int main() {
  tiny_ecu::RingBuffer<std::uint8_t, 4> buffer;

  assert(buffer.Empty());
  assert(!buffer.Full());
  assert(buffer.Size() == 0);
  assert(buffer.capacity() == 4);

  return 0;
}