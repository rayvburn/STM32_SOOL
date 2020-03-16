### STM32_SOOL - Semi-Object-Oriented library for STM32F1xx microcontrollers written in C

Tested thoroughly with the STM32F103C8T6-based board module (a.k.a. `Blue Pill`). I can't tell for other F1-series boards, but they should be compatible (at worst with small adjustments).

Development of `STM32_SOOL` library is assisted by `System Workbench for STM32` IDE, i.e. modified `Eclipse` IDE. All necessary instructions, how to import the library and useful Eclipse project files are included.

Most of the code is documented using Doxygen.

---

#### Main concepts:

—  	built on top of StdPeriph 3.5.0 and CMSIS libraries

— 	`private` fields of a `class` are marked with underscore `_` before the name, it is not recommended to operate on them directly in the infinite loop - instead of that use `class' methods`

— 	`inheritance` is done by putting a base class inside derived class'es structure - because of that a `base` name inside a `class` structure is reserved; unfortunately base `class` methods access is via `.` operator (like base.{FIELD}) which is not very convenient for multi-level inheritance

— 	all interrupt driven `classes` have implicit or explicit `volatile` specifier

— 	in case of derived classes which are interrupt-related one must remember to call base's interrupt handlers too

— 	InterruptHandler routines are designed to run IT flag check at the start and they always return 1 when flag was set (0 otherwise) and clear it. Returning an uint8_t value creates a possibility to run piece of code on certain interrupt event without bothering about routines used to clear other IT flags. Stardard IRQHandlers contain a set of if/else conditions one of which should always run when interrupt detected.

— 	when Base `class` and Derived one both have member functions named the same, in typical situation, only the one from Derived class needs to be called

— 	when processing interrupts if/else structure typically should be processed from the highest level `class` (the last level od derivation, selected as first IF), to the lowest (Base class)


