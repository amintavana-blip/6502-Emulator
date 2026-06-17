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

### 5. Jump to Subroutine (`INS_JSR`)
Transfers program control to a new execution target address after pushing the current return address (minus 1) onto the system stack. 
* **Syntax:** `JSR $8000`
* **Total Cycles:** 6
* **Formula:** $\text{Stack} \leftarrow \text{PC} - 1$, then $\text{PC} = \text{Target Address}$



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