# A-RISC (Aderyn's RISC), Very simple 32 bit architecture
- notice registers 4-31 are optional

# Register File
| Index | MNemonic | Description | width (bits)
|---|---|---|---|
| 0 | `$pr` | Program Register | 32 |
| 1..29/31 | `$rN` | Register | 32 |
| 30** for scrap mechanic | `$out` | Output Port (only in scrap mechanic) | 32 |
| 31** for scrap mechanic | `$in` | Input Port (only in scrap mechanic) | 32 |

# Flags
| MNemonic | Description |
|----------|-------------|
| `$ze`    | if result was zero |
| `$ov`| if result overflowed |
| `$cy`    | if the result was over 65535 |
| `$sg`| sign of the result |
| `$od` | was result odd |
| `$alw`| always |

# Opcode
| Code | MNemonic Prefix | Operation
|---|---|---|
| `0000` | `ADD` | `$rd <= $rs + $rs2 + $carry` |
| `0001` | `SUB` | `$rd <= $rs + ~$rs2 + !$carry` |
| `0010` | `EOR` | `$rd <= $rs ^ $rs2` |
| `0011` | `ORR` | `$rd <= $rs \| $rs2` |
| `0100` | `AND` | `$rd <= $rs & $rs2` |
| `0101` | `LLI` | `$rd.low <= i16` |
| `0110` | `LHI` | `$rd.hig <= i16` |
| `0111` | `Bcc` | `$pr <= $pr + $i23` conditionally (1) |
| `1000` | `L.32` | `$rd <= *($rs + i19)` (2) |
| `1001` | `S.32` | `*($rd + i19) <= $rs` (2) |
| `1010` | `L.16` | `$rd.low <= *($rs + i19).low` (2) |
| `1011` | `S.16` | `*($rd + i19).low <= $rs.low` (2) |
| `1100` | `L.8` | `$rs.low.low <= *($rd + i19).low.low` (2) |
| `1101` | `S.8` | `*($rd + i19).low.low <= $rs.low.low` (2) |
| `1110` | `***` | Reserved |
| `1111` | `LR` | `$rd <= $rs` |

## 1.
- use i4 in a multiplexer of the flags, if it is true, execute, otherwise, do not. (sign extend i23)
- all operatons change flags

## 2.
- this being an offset is optional

# opcode encoding
| bits | desc |
|------|------|
| `3:0` | opcode |
| `8:4` | $rd or i4 |
| `13:9` or `31:9` | $rs or `i23` |
| `31:13` | IF NOT ALU `i19` ELSE `rs2`, `i14` |

## note
- in Bcc, we use $rd as the i4, so we can have i23 aswell

- fixed 32 bits

# Memory
- memory is 8 bits wide, with 65536 elements

# Notes
- the intended implementation is inside a Game, (scrapmechanic), therefore, the upper **6** bits of memory access is ignored for the main implementation, but, you CAN use the whole address for your impl
- lowest bit of pr can be ignored, so, remember to align your instructions
- branch immediates are sign extended
- i23,i4 in branch,memory access is sign extended
- the carry for ADD,SUB,XOR,ORR,AND is always the carry of the ADD/SUB node.

# Implementation
- the design is very simple, i am not sure if all of the ops even work (lol)
![design](./design.png)
