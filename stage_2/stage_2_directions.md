# Tiny ECU Project - Stage 2 Assignment

## Stage 2 Title

UART Simulator + Volatile Experiment

## Context

You are continuing the Tiny ECU project from scratch for Stage 2.

You already have:

- Stage 0: Memory Foundations
- Stage 1: RingBuffer

In Stage 2, your job is to build a host-side UART simulator on your Mac. This prepares you for the later STM32 UART driver, but does not require hardware yet.

Important rule for this stage:

> You implement the UART simulator yourself, including the header and source files.

The RingBuffer implementation already exists and may be used.

---

# Part 1 - Objective

Build a `UartSimulator` class that models this embedded flow:

```text
Incoming UART bytes
        ↓
RingBuffer<uint8_t, 256>
        ↓
ReadByte()
        ↓
ReadLine()
        ↓
Future command parser
```

This simulates what a real embedded UART driver does:

```text
UART hardware receives byte
        ↓
Interrupt handler stores byte
        ↓
RingBuffer buffers byte
        ↓
Application/parser consumes byte
```

---

# Part 2 - What UART Means Here

UART is byte-stream communication.

If a user sends:

```text
GET STATUS\n
```

the system should think of this as individual bytes:

```text
'G'
'E'
'T'
' '
'S'
'T'
'A'
'T'
'U'
'S'
'\n'
```

A real UART does not deliver an entire command at once. It delivers bytes over time.

Your simulator should model that idea.

---

# Part 3 - Files To Create

Create these files:

```text
tiny_ecu/
  include/
    tiny_ecu/
      uart_simulator.h

  src/
    uart_simulator.cc

  tests/
    uart_simulator_test.cc

  experiments/
    volatile_experiment.cc

  docs/
    stage2_report.md
```

You may also update:

```text
BUILD.bazel
```

---

# Part 4 - Required Public API

Create a class named:

```cpp
tiny_ecu::UartSimulator
```

It must expose this public API:

```cpp
bool ReceiveByte(std::uint8_t byte);
bool ReceiveString(const std::string& input);

bool ReadByte(std::uint8_t* out);
bool ReadLine(std::string* out);

bool HasData() const;
std::size_t PendingBytes() const;
```

It should also define:

```cpp
static constexpr std::size_t kRxBufferSize = 256;
```

The class should internally use:

```cpp
RingBuffer<std::uint8_t, kRxBufferSize>
```

---

# Part 5 - Function Requirements

## 1. ReceiveByte

Purpose:

```text
Pretend one byte arrived from UART hardware.
```

Behavior:

```text
Input: one byte
Action: push byte into RX ring buffer
Return true if accepted
Return false if RX buffer is full
```

---

## 2. ReceiveString

Purpose:

```text
Convenience helper for tests.
```

Behavior:

```text
Treat the input string as a sequence of bytes.
Feed each character one at a time into ReceiveByte().
Return true only if all bytes were accepted.
Return false if any byte could not be accepted.
```

For this first version, it is acceptable if earlier bytes were already inserted before failure.

---

## 3. ReadByte

Purpose:

```text
Read one byte from the RX buffer.
```

Behavior:

```text
Return false if out == nullptr.
Return false if RX buffer is empty.
Otherwise pop one byte into *out and return true.
```

---

## 4. ReadLine

Purpose:

```text
Read a command line from the RX byte stream.
```

Behavior:

```text
Return false if out == nullptr.
Clear *out at the beginning.
Read bytes until '\n'.
Ignore '\r'.
Append all other bytes to *out.
Return true only if a newline was found.
```

Example:

```text
Input bytes:  SET SPEED 55\r\n
Output line:  SET SPEED 55
```

First-version rule:

```text
If no newline exists, partial bytes may be consumed.
```

This is okay for Stage 2. A later version may preserve partial lines.

---

## 5. HasData

Purpose:

```text
Tell whether RX buffer contains any bytes.
```

Behavior:

```text
Return true if at least one byte is pending.
Return false otherwise.
```

---

## 6. PendingBytes

Purpose:

```text
Report how many bytes are currently buffered.
```

Behavior:

```text
Return current RX buffer size.
```

---

# Part 6 - Implementation Order

Implement in this order:

```text
1. Header file
2. HasData
3. PendingBytes
4. ReceiveByte
5. ReadByte
6. ReceiveString
7. ReadLine
8. Tests
9. Volatile experiment
10. Stage 2 report
```

---

# Part 7 - Required Tests

Create `tests/uart_simulator_test.cc`.

Your tests must verify:

```text
1. A new UART simulator has no data.
2. PendingBytes() starts at 0.
3. ReadByte on an empty buffer returns false.
4. ReadByte(nullptr) returns false.
5. ReceiveByte('A') succeeds.
6. PendingBytes() becomes 1 after receiving one byte.
7. ReadByte returns the same byte that was received.
8. PendingBytes() returns to 0 after reading the byte.
9. ReceiveString("OK\n") succeeds.
10. ReadLine returns "OK".
11. ReceiveString("SET SPEED 55\r\n") succeeds.
12. ReadLine returns "SET SPEED 55".
13. ReadLine(nullptr) returns false.
```

Run:

```bash
bazel test //:uart_simulator_test
```

---

# Part 8 - Bazel Targets

Your `BUILD.bazel` should contain targets for:

```text
ring_buffer
uart_simulator
uart_simulator_test
volatile_experiment
```

Expected commands:

```bash
bazel test //:uart_simulator_test
bazel run //:volatile_experiment
```

---

# Part 9 - Volatile Experiment

Stage 2 also includes an experiment to understand:

```text
volatile vs std::atomic
```

Create:

```text
experiments/volatile_experiment.cc
```

This file is allowed to be provided/filled in separately because the learning goal is to run and observe it, not design the API.

The experiment should compare:

```text
plain bool
volatile bool
std::atomic<bool>
```

Learning goals:

```text
plain bool:
  wrong for cross-thread signaling
  has a data race

volatile bool:
  forces reloads
  useful mental model for hardware/ISR visibility
  still not thread-safe

std::atomic<bool>:
  correct for thread communication
```

Key takeaway:

```text
volatile is not a threading primitive.
volatile belongs near hardware registers and sometimes ISR-visible flags.
std::atomic belongs in thread synchronization.
```

---

# Part 10 - Stage 2 Report

Create:

```text
docs/stage2_report.md
```

Answer these like you are in an NVIDIA embedded panel interview.

## UART Design Questions

1. What did you build in Stage 2?
2. What is UART conceptually?
3. Why does UART naturally fit a RingBuffer?
4. Why are bytes received one at a time instead of as a full string?
5. What happens if bytes arrive faster than software consumes them?
6. Why not use `std::vector` or dynamic allocation for the RX buffer?
7. What is the time complexity of ReceiveByte and ReadByte?
8. What should happen when the ring buffer is full?
9. How would this change when implemented as a real STM32 UART ISR?
10. How would this change if using DMA?

## Volatile Questions

1. What does `volatile` mean?
2. What does `volatile` not guarantee?
3. Why is `volatile` not enough for thread synchronization?
4. Why is `std::atomic<bool>` better for cross-thread signaling?
5. Why will `volatile` matter later for STM32 memory-mapped registers?
6. What does this mean?

```cpp
volatile std::uint32_t* const uart_status;
```

Answer:

```text
Can the pointer change?
Can the register value change?
Why is volatile applied to the pointee?
Why is const applied to the pointer?
```

## Interview Defense Questions

Answer these as if a panelist is pushing back on your design:

1. Why did you put the RingBuffer inside UartSimulator instead of passing it in?
2. What are the tradeoffs of owning vs injecting the buffer?
3. What happens if ReadLine is called before a newline arrives?
4. Is your first ReadLine design production-safe?
5. How would you preserve partial lines in a future version?
6. What bugs could occur if ReceiveByte and ReadLine run concurrently?
7. Would you need a mutex, atomic indices, or interrupt disabling on real hardware?
8. What should never happen inside a real UART ISR?
9. What would you log or expose for debugging dropped bytes?
10. How would you test this without physical hardware?

---

# Part 11 - Success Criteria

Stage 2A is complete when:

```text
bazel test //:uart_simulator_test
```

passes.

Stage 2B is complete when:

```text
bazel run //:volatile_experiment
```

runs and you can explain:

```text
plain bool vs volatile bool vs std::atomic<bool>
```

Stage 2 overall is complete when:

```text
docs/stage2_report.md
```

answers the interview questions clearly.

---

# Part 12 - What To Paste Back For Review

When done, paste:

```text
include/tiny_ecu/uart_simulator.h
src/uart_simulator.cc
tests/uart_simulator_test.cc
docs/stage2_report.md
```

I will review:

```text
Correctness
C++ style
Google C++ style
Embedded design
Test quality
Interview readiness
```
