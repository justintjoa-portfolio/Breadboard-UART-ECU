#ifndef TINY_ECU_RING_BUFFER_H_
#define TINY_ECU_RING_BUFFER_H_

#include <array>
#include <cstddef>

namespace tiny_ecu {

template <typename T, std::size_t Capacity>
class RingBuffer {
 public:
  static_assert(Capacity > 0, "RingBuffer capacity must be greater than zero.");

  RingBuffer() = default;

  bool Push(const T& value) {
    if (Full()) {
      return false;
    }

    buffer_[head_] = value;
    head_ = NextIndex(head_);
    ++size_;
    return true;
  }

  bool Pop(T* out) {
    if (out == nullptr || Empty()) {
      return false;
    }

    *out = buffer_[tail_];
    tail_ = NextIndex(tail_);
    --size_;
    return true;
  }

  bool Empty() const { return size_ == 0; }

  bool Full() const { return size_ == Capacity; }

  std::size_t Size() const { return size_; }

  constexpr std::size_t capacity() const { return Capacity; }

 private:
  constexpr std::size_t NextIndex(std::size_t index) const {
    return (index + 1) % Capacity;
  }

  std::array<T, Capacity> buffer_{};
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t size_ = 0;
};

}  // namespace tiny_ecu

#endif  // TINY_ECU_RING_BUFFER_H_