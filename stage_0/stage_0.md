# Tiny ECU Project - Stage 0 Memory Foundations

## Purpose

Before building:

* RingBuffer
* UART Driver
* Scheduler
* Watchdog
* Memory Pool
* FreeRTOS Tasks
* STM32 Drivers

I need to understand:

* Stack
* Heap
* `.bss`
* `.data`
* `.rodata`
* `static`
* `const`
* `constexpr`
* `volatile`
* Storage duration
* Variable lifetime
* Symbolic references and relocation

The goal of Stage 0 is:

> Given any object or variable, determine:
>
> 1. Where it lives
> 2. How long it lives
> 3. Who initializes it

Important refinement:

> Not everything that occupies memory is a variable.
>
> Example: a string literal is an object in memory, but not a variable.

---

# Simplified Process Memory Layout

```text
+---------------------------------------+  High Addresses

| Environment Variables & CLI Arguments |
+---------------------------------------+

|                 STACK                 |  Grows downward
+---------------------------------------+

|                   |                   |
|                   v                   |
|                                       |
|                   ^                   |
|                   |                   |
+---------------------------------------+

|                 HEAP                  |  Grows upward
+---------------------------------------+

|          .bss (Zero-initialized)      |
+---------------------------------------+

|          .data (Initialized)          |
+---------------------------------------+

|         .rodata (Read-Only)           |
+---------------------------------------+

|          .text (Code segment)         |  Low Addresses
+---------------------------------------+
```

Note: exact memory layout depends on operating system, compiler, linker script, executable format, and platform. On STM32/microcontrollers, the linker script makes this much more explicit.

---

# Memory Regions

## Q1: What is `.bss`?

### Answer

`.bss` contains global and static variables without explicit initializers.

Example:

```cpp
int global;
static int counter;
```

Characteristics:

```text
Location: RAM
Initialization: Zeroed before main()
Lifetime: Entire program
```

Important:

> `.bss` does not mean "junk/uninitialized at runtime."
>
> It means "no explicit initializer in the source, so startup code zeroes it."

---

## Q2: What is `.data`?

### Answer

`.data` contains global and static variables with explicit initial values.

Example:

```cpp
int global = 5;
static int counter = 10;
```

Characteristics:

```text
Location: RAM
Initialization: Loaded/copied from executable image
Lifetime: Entire program
```

---

## Q3: What is `.rodata`?

### Answer

`.rodata` contains read-only data.

Examples:

```cpp
"hello"

const int lookup_table[] = {1, 2, 3};
```

Characteristics:

```text
Read-only
Typically stored in Flash/ROM or read-only memory pages
Lifetime: Entire program
```

---

## Q4: What is the stack?

### Answer

The stack stores automatic local variables.

Example:

```cpp
void foo()
{
    int local;
}
```

Characteristics:

```text
Created when function begins
Destroyed when function returns
Not automatically zeroed
```

---

## Q5: What is the heap?

### Answer

The heap stores dynamically allocated memory.

Example:

```cpp
int* p = new int(5);
```

Characteristics:

```text
Allocated with new/malloc
Destroyed with delete/free
Lifetime controlled by programmer or smart pointer owner
```

Embedded systems often avoid heap allocation because it can introduce fragmentation, leaks, and nondeterministic timing.

---

# Globals and Statics

## Q6: Where does an uninitialized global variable live?

```cpp
int global;
```

### Answer

```text
Location: .bss
Lifetime: Entire program
Initial Value: 0
```

---

## Q7: Where does an initialized global variable live?

```cpp
int global = 42;
```

### Answer

```text
Location: .data
Lifetime: Entire program
Initial Value: 42
```

---

## Q8: Where does an uninitialized static variable live?

```cpp
static int counter;
```

### Answer

```text
Location: .bss
Lifetime: Entire program
Initial Value: 0
```

---

## Q9: Where does an initialized static variable live?

```cpp
static int counter = 7;
```

### Answer

```text
Location: .data
Lifetime: Entire program
Initial Value: 7
```

---

## Q10: Why doesn't this live in `.rodata`?

```cpp
static int counter = 7;
```

### Answer

Because it can be modified:

```cpp
counter = 100;
```

`.rodata` is read-only.

Therefore this belongs in `.data`, not `.rodata`.

---

## Q11: Where does a static local variable live?

```cpp
void foo()
{
    static int counter;
}
```

### Answer

```text
Location: .bss
Lifetime: Entire program
Initial Value: 0
```

Even though it is declared inside a function, it has static storage duration.

---

## Q12: What is the lifetime of a static local variable?

```cpp
void foo()
{
    static int counter;
}
```

### Answer

Many people incorrectly answer:

```text
Until foo() returns
```

Correct answer:

```text
Entire program
```

It has function scope but static storage duration.

---

# Strings and Pointers

## Q13: Is a string literal a variable?

```cpp
"hello"
```

### Answer

No.

A string literal is an unnamed object in memory.

It has storage and lifetime, but it is not a variable because it has no variable name.

---

## Q14: Where does a string literal live?

```cpp
"hello"
```

### Answer

```text
Location: Usually .rodata
Lifetime: Entire program
Type: const char[6] in C++
Contents: h e l l o \0
```

---

## Q15: Analyze this declaration

```cpp
// assume global
const char* message = "hello";
```

### Answer

Two separate objects exist.

### Pointer variable

```cpp
message
```

```text
Location: .data
Lifetime: Entire program
Initial value at runtime: address of the first character of "hello"
```

### String literal

```cpp
"hello"
```

```text
Location: .rodata
Lifetime: Entire program
Value: {'h', 'e', 'l', 'l', 'o', '\0'}
```

---

## Q16: Why is the pointer in `.data`?

```cpp
const char* message = "hello";
```

### Answer

The pointer has an explicit initializer.

Conceptually, at runtime:

```cpp
message = &"hello"[0];
```

The pointer itself does not contain the string. It contains an address.

Anything requiring initialization data belongs in `.data`, not `.bss`.

---

## Q17: If the final address is not known during compilation, how is the pointer initialized?

### Answer

The compiler does not necessarily know the final numeric address at compile time.

Instead, the compiler emits a symbolic reference / relocation.

Process:

```text
Source
 ↓
Compiler
 ↓
Object File
 ↓
Linker
 ↓
Executable
 ↓
Loader
 ↓
Running Program
```

At compile time, the compiler knows something like:

```text
message should point to symbol/string-literal "hello"
```

It does not necessarily know:

```text
message == 0x104012340
```

The linker and/or loader later resolve the symbolic reference into a concrete address.

---

## Q18: Does the pointer become zero before relocation?

```cpp
const char* message = "hello";
```

### Answer

No.

It is not conceptually:

```text
message = 0
then later message = address_of_hello
```

Instead, the object file contains storage plus relocation metadata.

Conceptually:

```text
message:
    pointer-sized storage

relocation record:
    patch message with address of "hello"
```

The important part is not the placeholder bytes. The important part is the relocation record telling the linker/loader what address to patch in.

---

## Q19: What does the relocation look like?

### Answer

Object files contain sections and metadata.

Conceptually:

```text
.data:
    message: 8 bytes reserved

.rodata:
    .L_string_1: "hello\0"

.relocation:
    location: message
    symbol: .L_string_1
    type: pointer/address relocation
```

Think of it like a sticky note:

```text
Dear linker:
Replace these bytes with the address of "hello".
```

After linking/loading, `message` contains a real address.


## Q24: What is the difference between `const` and `constexpr`?

### Answer

```cpp
const int x = 42;
```

means:

```text
x cannot be modified.
```

```cpp
constexpr int x = 42;
```

means:

```text
x is known at compile time.
```

Interview shorthand:

```text
const answers: "Can I modify it?"
constexpr answers: "Can the compiler know it during compilation?"
```

---

# Stack and Heap

## Q25: Where does a local variable live?

```cpp
void foo()
{
    int local = 1;
}
```

### Answer

```text
Location: Stack
Lifetime: Until foo() returns
Initial Value: 1
```

---

## Q26: Why can local variables contain junk values?

```cpp
void foo()
{
    int x;
}
```

### Answer

The stack is not automatically zeroed.

The memory may contain leftover bytes from previous usage.

Reading an uninitialized local variable is undefined behavior.

Example:

```cpp
void task()
{
    bool watchdog_enabled;

    if (watchdog_enabled)
    {
        feed_watchdog();
    }
}
```

This is dangerous because `watchdog_enabled` contains an indeterminate value.

---

## Q27: Why are globals and statics zeroed automatically?

### Answer

Startup code clears `.bss` before `main()`.

Conceptually:

```cpp
memset(bss_start, 0, bss_size);
```

This guarantees all `.bss` variables start as zero.

---

# Volatile

## Q31: What does `volatile` mean?

### Answer

`volatile` tells the compiler:

> This value may change without a visible write in the current thread of execution.

Example:

```cpp
volatile uint32_t UART_STATUS;
```

The compiler should reread it instead of assuming the value is unchanged.

---

## Q32: Does `volatile` determine where memory lives?

### Answer

No.

`volatile` affects compiler behavior.

It does not determine:

* stack
* heap
* `.bss`
* `.data`
* `.rodata`

A volatile object could be global, local, static, or memory-mapped hardware.

---

## Q33: What are the two most common embedded uses of `volatile`?

### Hardware registers

```cpp
volatile uint32_t UART_STATUS;
```

Hardware can modify the value.

### ISR communication

```cpp
volatile bool data_ready;
```

An interrupt handler can modify the value.

---

## Q34: Does `volatile` make code thread-safe?

### Answer

No.

`volatile`:

* does not prevent races
* does not provide atomicity
* does not provide synchronization

Use:

```cpp
std::atomic
std::mutex
std::condition_variable
```

for synchronization.

---

# Embedded-Specific Questions

## Q35: Why do embedded systems often avoid heap allocation?

### Answer

Heap allocation can cause:

* fragmentation
* memory leaks
* nondeterministic timing
* allocation failure at runtime

Embedded systems often prefer:

```cpp
static uint8_t rx_buffer[256];
```

because memory usage is known ahead of time.

---

## Q36: Compare these

```cpp
static uint8_t buffer[100000];
```

vs

```cpp
static uint8_t buffer[100000] = {1};
```

### Answer

The first typically belongs in:

```text
.bss
```

It requires RAM at runtime, but little/no explicit initialization data in the executable.

The second requires initialization information because at least part of the object has a nonzero initial value.

The exact section placement can depend on toolchain and linker behavior, but the important embedded takeaway is:

> Explicit nonzero initialization generally increases initialization data requirements compared to `.bss`.


## Q39: Why does `const` placement matter for pointers?

### Answer

With pointers, `const` can apply to:

1. The object being pointed to
2. The pointer variable itself
3. Both

A useful reading rule:

> Read from right to left around the `*`.

---

### Pointer to const data

```cpp
const int* p;
```

Same as:

```cpp
int const* p;
```

Meaning:

```text
p points to an int that cannot be modified through p.
p itself can be changed to point somewhere else.
```

Example:

```cpp
int x = 1;
int y = 2;

const int* p = &x;
p = &y;      // OK
*p = 10;     // Error
```

---

### Const pointer to mutable data

```cpp
int* const p = &x;
```

Meaning:

```text
p itself cannot be changed.
The int it points to can be modified through p.
```

Example:

```cpp
int x = 1;
int y = 2;

int* const p = &x;
*p = 10;     // OK
p = &y;      // Error
```

---

### Const pointer to const data

```cpp
const int* const p = &x;
```

Same idea as:

```cpp
int const* const p = &x;
```

Meaning:

```text
p cannot be changed.
The int cannot be modified through p.
```

Example:

```cpp
int x = 1;
int y = 2;

const int* const p = &x;
*p = 10;     // Error
p = &y;      // Error
```

---

## Q40: How does this apply to strings?

```cpp
const char* message = "hello";
```

### Answer

This means:

```text
message is a pointer to const char.
```

So:

```cpp
message = "world";   // OK
message[0] = 'H';    // Error
```

The pointer variable can point somewhere else, but the characters in the string literal should not be modified.

There are two separate things:

```text
message:
    pointer variable
    usually .data if global
    initialized to address of "hello"

"hello":
    string literal
    usually .rodata
    read-only
```

---

## Q41: What if the pointer itself is const?

```cpp
const char* const message = "hello";
```

### Answer

This means:

```text
message is a const pointer to const char.
```

So:

```cpp
message = "world";   // Error
message[0] = 'H';    // Error
```

If this is global or namespace-scope, the pointer may be placed in read-only storage because both the pointer and the pointed-to characters are immutable.

Conceptually:

```text
message:
    cannot be reseated
    points to "hello"

"hello":
    cannot be modified
```

---

## Q42: What is the embedded takeaway?

### Answer

For embedded code, always ask two separate questions:

```text
Can I change the pointer?
Can I change the thing it points to?
```

Examples:

```cpp
volatile uint32_t* uart_status;
```

Pointer to a volatile hardware register. The pointer can change.

```cpp
volatile uint32_t* const uart_status;
```

Const pointer to a volatile hardware register. The pointer cannot change, but the register value can change due to hardware.

This is common for memory-mapped I/O.

---

## Q43: How should I remember pointer const syntax?

### Answer

Use this table:

```text
const int* p
    Pointer can change.
    Data cannot be changed through p.

int* const p
    Pointer cannot change.
    Data can be changed through p.

const int* const p
    Pointer cannot change.
    Data cannot be changed through p.
```

For Tiny ECU, this will matter when writing driver APIs and hardware register definitions.


---

# Stage 0 Completion Criteria

I can correctly determine:

* Stack vs Heap
* `.bss` vs `.data` vs `.rodata`
* static storage duration
* automatic storage duration
* dynamic storage duration
* `const` vs `constexpr`
* `volatile` usage
* pointer location vs pointed-to object location
* string literal vs variable
* symbolic relocation vs final runtime address

without looking at notes.

At that point I am ready for:

```text
Stage 1 - RingBuffer
```
