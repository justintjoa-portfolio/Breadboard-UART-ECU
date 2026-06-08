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

---

# Compile-Time Constants, Const, and Linkage

## Q20: Is `const` the same as a compile-time constant?

### Answer

No.

`const` means:

```text
This object cannot be modified through this name.
```

It does not always mean:

```text
The compiler knows the value at compile time.
```

Example:

```cpp
const int x = get_value();
```

`x` is const, but its value is only known after `get_value()` runs.

---

## Q21: What is a compile-time constant?

### Answer

A compile-time constant is a value the compiler can fully determine during compilation.

Modern C++ usually expresses this with `constexpr`.

Example:

```cpp
constexpr int BufferSize = 256;
```

The compiler knows:

```text
BufferSize == 256
```

during compilation.

This can be used for templates, array sizes, and optimization.

Tiny ECU example:

```cpp
constexpr std::size_t UartRxBufferSize = 256;
```

---

## Q22: What about a const variable in an unnamed namespace?

```cpp
namespace
{
    const int kValue = 42;
}
```

### Answer

Formal classification:

```text
Namespace-scope const object
Static storage duration
Internal linkage
Usually stored in .rodata
```

The unnamed namespace affects linkage/visibility, not lifetime.

The `const` affects mutability.

The namespace-scope placement gives it static storage duration.

---

## Q23: Is this a "build-time constant"?

```cpp
namespace
{
    const int kValue = 42;
}
```

### Answer

"Build-time constant" is informal wording, not the clearest C++ term.

Better terms:

```text
const object
static storage duration
internal linkage
possibly usable as a constant expression depending on context
```

If the goal is to force compile-time evaluation, prefer:

```cpp
namespace
{
    constexpr int kValue = 42;
}
```

Then it is a constant expression.

---

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

## Q28: Analyze this declaration

```cpp
int* p = new int(5);
```

### Answer

Two objects exist.

### Pointer variable

```cpp
p
```

```text
Location: Stack if local
Lifetime: Until function returns
```

### Integer object

```cpp
new int(5)
```

```text
Location: Heap
Lifetime: Until delete
```

Always separate:

```text
The pointer variable
```

from:

```text
The object being pointed to
```

---

## Q29: Does heap memory persist if `delete` is never called?

```cpp
int* p = new int(5);
```

### Answer

Yes.

The object remains alive until:

```cpp
delete p;
```

or until the process exits.

Failing to free memory is a memory leak.

On a desktop OS, the OS reclaims memory when the process exits. That does not help a long-running program while it is still running.

---

## Q30: Analyze this declaration

```cpp
auto p = std::make_unique<int>(42);
```

### Answer

### Smart pointer variable

```cpp
p
```

```text
Location: Stack if local
Lifetime: Until scope exits
```

### Integer object

```cpp
int(42)
```

```text
Location: Heap
Lifetime: Until p is destroyed
```

When `p` goes out of scope, the heap object is automatically deleted.

This is RAII.

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

---

## Q37: Tiny ECU UART buffer example

```cpp
static RingBuffer<uint8_t, 256> uart_rx_buffer;
```

### Answer

```text
Location: .bss, assuming zero initialization and no nontrivial constructor requiring dynamic initialization
Lifetime: Entire program
Initialization: Zero-initialized before main()
```

This is a common embedded pattern.

It avoids:

* heap allocation
* repeated stack allocation
* lifetime bugs

---

## Q38: Tiny ECU watchdog example

```cpp
static Watchdog watchdog;
```

### Answer

If `Watchdog` is zero-initialized and has no nontrivial dynamic initialization requirements:

```text
Location: typically .bss
Lifetime: Entire program
Initialization: before main()
```

This matters because the watchdog should survive for the lifetime of the ECU.

---

## Q39: Tiny ECU pointer example

```cpp
static RingBuffer<uint8_t, 256> uart_rx_buffer;
RingBuffer<uint8_t, 256>* ptr = &uart_rx_buffer;
```

### Answer

There are two objects:

```text
uart_rx_buffer:
    Location: typically .bss
    Lifetime: entire program

ptr:
    Location: .data
    Lifetime: entire program
    Initial value at runtime: address of uart_rx_buffer
```

At compile time, the pointer may be represented as a relocation to the symbol `uart_rx_buffer`.

---

# Stage 0 Master Rule

For every object or variable ask:

1. Where does it live?
2. How long does it live?
3. Who initializes it?

Also ask:

4. Is it mutable or read-only?
5. Is the value known at compile time, link time, load time, or only runtime?
6. Am I talking about the pointer variable or the object being pointed to?

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

# Memory Faults: Out-of-Bounds Access vs Stack Overflow vs Segmentation Fault

## Q44: What Is a Segmentation Fault?

### Definition

A segmentation fault is an operating-system fault that occurs when a process attempts to access memory that it is not permitted to access.

Examples:

```cpp
int* p = nullptr;
*p = 42;
```

```cpp
delete p;
*p = 42;
```

```cpp
int* p = reinterpret_cast<int*>(0x1234);
*p = 42;
```

Common causes:

```text
Null pointer dereference
Use-after-free
Out-of-bounds access
Invalid pointer arithmetic
Writing to read-only memory
Stack overflow
```

On Linux/macOS:

```text
CPU detects invalid memory access
↓
Operating system receives fault
↓
Process terminated
↓
Segmentation Fault
```

---

## Q44: What Is a Stack Overflow?

### Definition

A stack overflow occurs when the stack exceeds its available memory.

Common causes:

### Infinite Recursion

```cpp
void Recurse() {
    Recurse();
}
```

### Excessive Stack Allocation

```cpp
void Foo() {
    int huge_buffer[10000000];
}
```

The root problem is:

```text
Too much stack memory consumed
```

---

## Q45: What Is an Out-of-Bounds Access?

### Definition

An out-of-bounds access occurs when code accesses memory outside the valid range of an object.

Example:

```cpp
int arr[10];
arr[100] = 42;
```

Valid indices are:

```text
0 through 9
```

Therefore:

```cpp
arr[100]
```

is an:

```text
Out-of-bounds access
```

and therefore:

```text
Undefined Behavior
```

according to the C++ language.

---

## Q46: What Is the Difference Between These Three Concepts?

### Answer

These are three different layers of the system.

### Layer 1: C++ Language Rules

```text
Out-of-bounds access
```

Example:

```cpp
int arr[10];
arr[100] = 42;
```

This violates C++ object bounds.

The language says:

```text
Undefined Behavior
```

---

### Layer 2: Program Behavior

Undefined behavior may produce:

```text
Memory corruption
Incorrect results
Silent failure
Crash
```

---

### Layer 3: Operating System Protection

If the invalid access reaches memory that the process does not own:

```text
CPU fault
↓
Operating System fault
↓
Segmentation Fault
```

---

## Q47: Why Doesn't Every Out-of-Bounds Access Cause a Segmentation Fault?

### Answer

Because:

```text
Array bounds are a C++ concept.

Memory permissions are an OS/hardware concept.
```

The CPU does not know:

```text
This array has length 10.
```

The CPU only knows:

```text
Read address X
Write address Y
```

The operating system only checks:

```text
Does this process own this memory?
```

It does NOT check:

```text
Is this access inside the array bounds?
```

---

## Example

```cpp
void Foo() {
    int a = 1;
    int arr[10];
    int b = 2;

    arr[15] = 42;
}
```

Conceptually:

```text
Stack

+---------+
|    a    |
+---------+
| arr[0]  |
| arr[1]  |
| ...     |
| arr[9]  |
+---------+
|    b    |
+---------+
```

Suppose:

```cpp
arr[15] = 42;
```

overwrites:

```text
b
another local variable
saved registers
```

The CPU sees:

```text
Write to memory owned by the process
```

and allows it.

Result:

```text
Out-of-bounds access
Memory corruption
No segmentation fault
```

---

## Q48: When Does an Out-of-Bounds Access Cause a Segmentation Fault?

### Answer

When the invalid access reaches memory the process is not permitted to access.

Example:

```cpp
int* p = nullptr;
*p = 42;
```

or:

```cpp
int arr[10];
arr[1000000000] = 42;
```

Eventually:

```text
Invalid memory access
↓
CPU fault
↓
Segmentation Fault
```

---

## Q49: How Are Stack Overflows Related to Segmentation Faults?

### Answer

A stack overflow is a specific bug.

A segmentation fault is often the resulting symptom.

```text
Stack Overflow
        ↓
Invalid Memory Access
        ↓
Segmentation Fault
```

This is common on Linux and macOS because the operating system protects memory regions.

---

## Q50: Which Concept Is More General?

### Answer

Segmentation fault is the most general concept.

Think of it this way:

```text
Segmentation Faults
├── Null pointer dereference
├── Use-after-free
├── Invalid pointer arithmetic
├── Out-of-bounds access
└── Stack overflow
```

Therefore:

```text
Many stack overflows eventually manifest as segmentation faults.

Many out-of-bounds accesses eventually manifest as segmentation faults.

Not all segmentation faults are stack overflows.

Not all segmentation faults are caused by out-of-bounds accesses.
```

---

## Q51: What Is the Correct Mental Model?

### Answer

```text
Out-of-Bounds Access
        ↓
Undefined Behavior
        ↓
Maybe Segmentation Fault
```

```text
Stack Overflow
        ↓
Invalid Memory Access
        ↓
Often Segmentation Fault
```

```text
Null Pointer Dereference
        ↓
Invalid Memory Access
        ↓
Segmentation Fault
```

The key idea:

```text
Out-of-bounds access
    = Bug

Stack overflow
    = Bug

Segmentation fault
    = Common runtime symptom
```

---

## Embedded Systems Note

On Linux/macOS:

```text
Memory protection exists.
```

Therefore many memory bugs become:

```text
Segmentation Fault
```

On many microcontrollers:

```text
No MMU
No virtual memory
Little or no memory protection
```

Therefore:

```cpp
arr[1000] = 42;
```

may instead produce:

```text
Memory corruption
Corrupted globals
Corrupted stack
HardFault exception
Watchdog reset
Random behavior
```

without a segmentation fault.

---

## Interview Answer

If asked:

"What's the difference between an out-of-bounds access, a stack overflow, and a segmentation fault?"

A strong answer is:

```text
An out-of-bounds access is a C++ language-level bug where code accesses memory outside the bounds of an object.

A stack overflow is a specific bug where the stack exceeds its available memory.

A segmentation fault is an operating-system fault that occurs when a process accesses memory it is not permitted to access.

Both stack overflows and out-of-bounds accesses can lead to segmentation faults, but neither guarantees one. A segmentation fault is generally the symptom, while the underlying bug is often an out-of-bounds access, a stack overflow, a null pointer dereference, or another memory error.
```



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
