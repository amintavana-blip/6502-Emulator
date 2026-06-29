# 🚀 6502 Emulator Architecture Guide

---

## 💾 Addressing Modes Reference

### 1. Immediate Mode (`INS_LDA_IM`)
Loads a raw 8-bit constant value directly into the accumulator.
* **Syntax:** `LDA #$10`
* **Cycles:** 2

```cpp
Byte Value = FetchByte(cycles, memory);
A = Value;
```
---

### 2. Zero Page Mode (`INS_LDA_ZP`)
Fetches an 8-bit memory address pointer from the program stream, then reads the data from that location in the first 256 bytes of RAM (Page 0).
* **Syntax:** `LDA $42`
* **Total Cycles:** 3
* **Formula:** $\text{Target Address} = \text{Fetched Byte}$

```cpp
Byte ZeroPageAddress = FetchByte(cycles, memory);
A = ReadByte(cycles, ZeroPageAddress, memory);
```

---

### 3. Zero Page, X Mode (`INS_LDA_ZPX`)
Fetches an 8-bit memory address pointer, then adds the value of the X register to it. This acts as an array offset, allowing you to loop through sequential lists of data in memory.
* **Syntax:** `LDA $42,X`
* **Total Cycles:** 4
* **Formula:** Target Address = (Fetched Byte + X) % 256

```cpp
Byte ZeroPageAddress = FetchByte(cycles, memory);
ZeroPageAddress += X; // Offset address by X register
cycles--;             // Internal calculation costs 1 cycle
A = ReadByte(cycles, ZeroPageAddress, memory);

```
---

## 🎬 The Ultimate Concept Guide: Fetching vs. Reading

If computer architecture feels abstract, think of the system as a **Feature Film**:

### 📸 A "Read" is a Single Picture (A Frame of Film)
* It is a solitary, raw mechanical action. 
* The CPU puts an address on the bus, and RAM hands back a byte. 
* By itself, a single read has no context—it’s just a snapshot of a number sitting on a wire.

### 🎬 A "Fetch" is a Specific Scene (A Sequence with Purpose)
* A fetch is a **Read with a very strict destination and mission**. 
* It is the CPU intentionally reading the next piece of its script using the Program Counter (`PC`).
* This data is delivered directly into the CPU's brain (the Instruction Register) to advance the plot of the program.

---

### 🗺️ Case Study: Breaking Down `INS_LDA_ZPX`

Here is how your "Movie" executes this exact sequence step-by-step:

1. **The Fetch (Reading the Script):** The CPU runs `FetchByte`. It looks at the `PC`, pulls a byte out of memory, and discovers it is a base address pointer (e.g., `0x42`).
2. **The Intermission (The Math):** The CPU freezes its memory lines for 1 clock cycle (`cycles--`) to physically calculate `0x42 + X` inside its hardware transistors. The resulting sum happens to be our final target address (e.g., `0x47`).
3. **The Read (Snapping the Picture):** The CPU executes a generic `ReadByte` at that calculated address (`0x47`). It snaps a picture of that RAM slot, grabs the actual data payload inside, and drops it straight into the Accumulator register (`A`).

---

### 4. Absolute Addressing Mode (`INS_LDA_ABS`)
Fetches a full 16-bit address pointer directly out of the program stream (low byte first, then high byte), then reads the data value stored at that specific system coordinate.
* **Syntax:** `LDA $8000`
* **Total Cycles:** 4
* **Formula:** $\text{Target Address} = \text{Fetched Word (16-bit)}$

```cpp
Word AbsoluteAddress = FetchWord(cycles, memory);
A = ReadByte(cycles, AbsoluteAddress, memory);
LDASetStatus();

---
```

## 5. Absolute X (`ABSX`) Addressing Mode

**Absolute X** is an indexed addressing mode that allows the CPU to dynamically target a memory location by combining a fixed 16-bit reference point with the contents of the **X Register**. 

It is one of the most powerful modes on the 6502 for processing linear data structures, like arrays or game graphics tables.

---

### a. How It Works (The Execution Steps)

When the CPU encounters an instruction like `LDA $2000,X`, it performs a 3-step calculation to find the final target address:

1. **Fetch the Base Address:** The instruction supplies a full 16-bit **Base Address** (e.g., `$2000`). This is the fixed starting anchor of your data structure.
2. **Inject the Offset:** The CPU reads the current 8-bit value inside the **X Register** (e.g., `X = $05`). This acts as the index pointer.
3. **Generate Target Address:** The CPU adds the offset to the base address to pinpoint the exact data payload slot:

   $$\text{Base Address (\$2000)} + \text{Offset Register (\$05)} = \mathbf{\text{Effective Address (\$2005)}}$$

---

### b. Memory Structure & The Page-Crossing Penalty

Because the 6502's 16-bit memory space is split into individual **256-byte pages**, adding an 8-bit offset to a base address can sometimes cause the calculation to spill over into the next page. 

* **No Page Cross (4 Cycles):** If the addition stays within the same page boundary, the instruction finishes in its standard **4-cycle** time block.
* **Page Cross (5 Cycles):** If the addition forces the low byte to overflow (exceeding `$FF`), the high byte must increment by 1. Real 6502 hardware requires **1 extra penalty cycle** to stabilize the address bus high byte, pushing the execution to **5 cycles**.

#### Practical Math Example:
* **Base Address:** `$2070` (High Byte/Page = `$20`)
* **X Register:** `$92`
* **Calculation:** `$2070 + $92 = $2102`
* **Result:** The page shifted from `$20` to `$21`. **Penalty triggered (+1 cycle).**

---

### c. Quick Reference Matrix

| Feature | Specification |
| :--- | :--- |
| **Instruction Size** | 3 Bytes (1 Opcode + 2 Base Address Bytes) |
| **Base Timing** | 4 Clock Cycles |
| **Page-Cross Timing** | 5 Clock Cycles (+1 penalty cycle) |
| **Common Use Case** | Iterating through arrays, reading tables, or clearing blocks of RAM. |

---

### d. Implementation Blueprint (C++)

Inside our emulator, the logic mimics the address bus comparison directly before reading the final data byte:

```cpp
Word BaseAddress = FetchWord(cycles, memory);    // Read 16-bit base
Word TargetAddress = BaseAddress + X;            // Add 8-bit offset

// Check if the page (high byte) changed
if ((BaseAddress & 0xFF00) != (TargetAddress & 0xFF00))
{
    cycles--; // Burn 1 extra cycle for the page cross stabilization
}

Byte FinalValue = ReadByte(cycles, TargetAddress, memory);

### 5. Jump to Subroutine (`INS_JSR`)
Transfers program control to a new execution target address after pushing the current return address (minus 1) onto the system stack. 
* **Syntax:** `JSR $8000`
* **Total Cycles:** 6
* **Formula:** $\text{Stack} \leftarrow \text{PC} - 1$, then $\text{PC} = \text{Target Address}$

---

```cpp

case INS_JSR:
{
    // 1. Fetch the 16-bit destination address where we want to jump
    Word SubAddress = FetchWord(cycles, memory);

    // 2. Calculate the return point minus 1
    Word ReturnPointMinusOne = PC - 1;

    // 3. Write this 16-bit address onto the stack
    // Current setup uses high-level WriteWord abstraction tracking (Little-Endian)
    WriteWord(cycles, ReturnPointMinusOne, STACK_PAGE_BASE + SP, memory);
    SP -= 2; 
    PC = SubAddress;

    cycles--; // JSR requires 1 final internal cycle to finish altering hardware paths

    /* -------------------------------------------------------------------------
       💡 HARDWARE-ACCURATE ALTERNATIVE (MANUAL SPLIT)
       The actual 6502 chip is a downward-growing "Qandil" (stalactite) stack. 
       It splits the word and pushes the High Byte first before decrementing. 
       If switching to raw bytes, comment out the WriteWord block above and use:

       WriteByte(cycles, (ReturnPointMinusOne >> 8) & 0xFF, STACK_PAGE_BASE + SP, memory);
       SP--;
       WriteByte(cycles, ReturnPointMinusOne & 0xFF, STACK_PAGE_BASE + SP, memory);
       SP--;
       PC = SubAddress;
       ------------------------------------------------------------------------- */

    break;
}

```