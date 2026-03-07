# A-RISC/32 (Aderyn's RISC/32), Very simple 32 bit architecture
- notice registers 4-31 are optional

# Register File
| Index | MNemonic | Description | width (bits)
|---|---|---|---|
| 0 | `$pr` | Program Register | 32 |
| 1 | `$lr` | Link Register | 32 |
| 2 | `$sr` | Stack Register | 32 |
| 3..31 | `$rN` | Register | 32 |

- also `$pr'`, `$cs'` and `$flags'`

# Flags
| MNemonic | Description |
|----------|-------------|
| `$ze`    | if result was zero |
| `$ov`| if result overflowed |
| `$cy`    | if the result was over 2^32 - 1 |
| `$sg`| sign of the result |
| `$od` | was result odd |
| `$alw`| always |
| `$usr` | User Mode |
| `$big` | Using Big Endian |

# Segmentation
- im too lazy to make page tables, so this is better
| MNemonic | Description |
|----------|-------------|
| `$cs`    | current segment, 8 bit |

- in the processor we basically have
```rust
struct Segment
{
        phys_base: u32
        phys_length: u28,
        execute: bool, read: bool, write: bool, prot: bool
}

segments: [Segments; 256];
```

## prot
- if `$usr` and accessing a segment with `prot=1`, interrupt but use address 64

# Opcode
| Code | MNemonic Prefix | Operation
|---|---|---|
| `0000` | `ADD` | `$rd <= $rs + $rs2 + $carry` |
| `0001` | `SUB` | `$rd <= $rs + ~$rs2 + !$carry` |
| `0010` | `EOR` | `$rd <= $rs ^ $rs2` |
| `0011` | `ORR` | `$rd <= $rs \| $rs2` |
| `0100` | `AND` | `$rd <= $rs & $rs2` |
| `0101` | `LI` | `$rd <= i23` |
| `0110` | `Bnc` | `$pr <= $pr + $i23, $lr <= $old_pr` conditionally (1) (but invert) |
| `0111` | `Bcc` | `$pr <= $pr + $i23, $lr <= $old_pr` conditionally (1) |
| `1000` | `L.32` | `$rd <= *i8:($rs + i10)` (2) |
| `1001` | `S.32` | `*i8:($rd + i10) <= $rs` (2) |
| `1010` | `L.16` | `$rd.low <= *i8:($rs + i10).low`, rest of bits 0  (2) |
| `1011` | `S.16` | `*i8:($rd + i10).low <= $rs.low`, rest of bits 0  (2) |
| `1100` | `L.8` | `$rd.low.low <= *i8:($rs + i10).low.low`, rest of bits 0 (2) |
| `1101` | `S.8` | `*i8:($rd + i10).low.low <= $rs.low.low`, rest of bits 0 (2) |
| `1110` | `EXT` | Extend |
| `1111` | `LR` | `$rd <= $rs` |

## Extend

| sub-op | mnemonic | function |
|--------|----------|----------|
|`000000`| `PUSH`   | `*($sr) <= $rs, $sr -= 4` |
|`000001`| `POP`   | `$sr += 4, rd <= *($sr)` |
|`000000`| `AL.32` | `$rd <= *($rs + i8)` (2), but set the BLOCK output pin high to block other cores |
|`000001`| `AS.32` | `*($rd + i8) <= $rs` (2), but set the BLOCK output pin high to block other cores |
|`000010`| `AL.16` | `$rd.low <= *($rs + i8).low` (2), but set the BLOCK output pin high to block other cores |
|`000011`| `AS.16` | `*($rd + i8).low <= $rs.low` (2), but set the BLOCK output pin high to block other cores |
|`000100`| `AL.8` | `$rd.low.low <= *($rs + i8).low.low` (2), but set the BLOCK output pin high to block other cores |
|`000101`| `AS.8` | `*($rd + i8).low.low <= $rs.low.low` (2), but set the BLOCK output pin high to block other cores |
|`000110`| `SSEG` | `segments($rd) <= $rs, $rs2`, nop in usermode |
|`000111`| `GSEG` | `$rd, $rs2 <= segments($rd)`, nop in usermode |
|`001000`| `BFAR` | `$pr, $cs <= i19, $rs` nop in usermode |
|`001001`| `SYSENTER` | `r30, r31, r29 <= $pr, $cs, $flags, <= $pr, $cs <= 16, 0, $usr <= 0` trashes r29, r30, r31 |
|`001010`| `SYSEXIT` | `$pr, $cs <= r30, r31, r29, $usr <= 1` nop in usermode |
|`001011`| `IRET` | `$pr, $cs, $flags <= $pr', $cs', $flags'` nop in usermode |
|`001100`| `BIG` | `$big <= 1` |
|`001101`| `LITTLE` | `$big <= 0` |

## 1.
- use i5 in a multiplexer of the flags, if it is true, execute, otherwise, do not. (sign extend i23)
- all operatons change flags

## 2.
- this being an offset is optional

# Interrupts
- uh `$pr'`, `$cs'`, `$flags'` <= `$pr`, `$cs`, `$flags` and `$pr`, `$cs` <= `32`, `0`

# opcode encoding (ADD, SUB, EOR, ORR, AND, SSEG, GSEG, LR, PUSH, POP, EXT)
| bits | desc |
|------|------|
| `3:0` | opcode |
| `8:4` | `$rd` |
| `13:9` | `$rs` |
| `18:14` | `$rs2` |
| `21:19` | `$cond` |
| `22` | invert `$cond` |
| `25:23` | special mode |
| `31:26` | special data, or EXT function |

# opcode encoding (L.xx, S.xx, AL.xx, AS.xx)
| bits | desc |
|------|------|
| `0:3`| opcode |
| `8:4` | `$rd` |
| `13:9` | `$rs` |
| `21:14` | i8 |
| `31:22` | i10 |

# opcode encoding (LI)
| bits | desc |
|------|------|
| `0:3`| opcode |
| `8:4`| `$rd` |
| `31:9` | i23|

# opcode encoding

## note
- in Bcc, we use $rd as the i5, so we can have i23 aswell
- fixed 32 bit opcode length

# Memory
- memory is 8 bits wide, with up to 2^32 elements

# Notes
- branch immediates are sign extended
- i23,i5 in branch,memory access is sign extended
- the carry for ADD,SUB,XOR,ORR,AND is always the carry of the ADD/SUB node.

# Implementation
- TODO

## Vectors
| adr | function |
|------|----------|
| `0`  | reset |
| `16` | sysenter |
| `32` | hardware interrupt |
| `48` | hardware interrupt 2 |
| `64` | protection fault |

# Endianness
- Variable, based on the $big flag
