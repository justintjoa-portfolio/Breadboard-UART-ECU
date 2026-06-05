#ifndef TINY_ECU_RING_BUFFER_H_
#define TINY_ECU_RING_BUFFER_H_

#include <array>
#include <cstddef>

namespace tiny_ecu {

template <typename T, std::size_t Capacity>
class RingBuffer {
 public:
  RingBuffer() = default;

  bool Push(const T& value);
  bool Pop(T* out);

  bool Empty() const;
  bool Full() const;

  std::size_t Size() const;
  constexpr std::size_t capacity() const { return Capacity; }

 private:
  std::array<T, Capacity> buffer_{};
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t size_ = 0;
};

}  // namespace tiny_ecu

#endif  // TINY_ECU_RING_BUFFER_H_