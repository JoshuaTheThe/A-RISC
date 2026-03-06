# A-RISC/32 (Aderyn's RISC/32), Very simple 32 bit architecture
- notice registers 4-31 are optional

# Register File
| Index | MNemonic | Description | width (bits)
|---|---|---|---|
| 0 | `$pr` | Program Register | 32 |
| 1 | `$lr` | Link Register | 32 |
| 2 | `$sr` | Stack Register | 32 |
| 3..31 | `$rN` | Register | 32 |

# Flags
| MNemonic | Description |
|----------|-------------|
| `$ze`    | if result was zero |
| `$ov`| if result overflowed |
| `$cy`    | if the result was over 65535 |
| `$sg`| sign of the result |
| `$od` | was result odd |
| `$alw`| always |
| `$usr` | User Mode |

# Segmentation
- im too lazy to make page tables, so this is better
| MNemonic | Description |
|----------|-------------|
| `$cs`    | current segment, 8 bit |

- in the processor we basically have
```rust
struct Segments
{
        phys_bases: [u32; 256],
        phys_lengths: [u32; 256],
}

segments: Segments;
```

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
| `1000` | `L.32` | `$rd <= *($rs + i19)` (2) |
| `1001` | `S.32` | `*($rd + i19) <= $rs` (2) |
| `1010` | `L.16` | `$rd.low <= *($rs + i19).low`, rest of bits 0  (2) |
| `1011` | `S.16` | `*($rd + i19).low <= $rs.low`, rest of bits 0  (2) |
| `1100` | `L.8` | `$rd.low.low <= *($rs + i19).low.low`, rest of bits 0 (2) |
| `1101` | `S.8` | `*($rd + i19).low.low <= $rs.low.low`, rest of bits 0 (2) |
| `1110` | `EXT` | Extend |
| `1111` | `LR` | `$rd <= $rs` |

## Extend

| sub-op | mnemonic | function |
|--------|----------|----------|
|`000000`| `PUSH`   | `*($sr) <= $rs, $sr -= 4` |
|`000001`| `POP`   | `$sr += 4, rd <= *($sr)` |
|`000010`| `Rnc` | `$pr <= $lr` conditionally (1) (but invert) |
|`000011`| `Rcc` | `$pr <= $lr` conditionally (1) |
|`000100`| `AL.32` | `$rd <= *($rs + i8)` (2), but set the BLOCK output pin high to block other cores |
|`000101`| `AS.32` | `*($rd + i8) <= $rs` (2), but set the BLOCK output pin high to block other cores |
|`000110`| `AL.16` | `$rd.low <= *($rs + i8).low` (2), but set the BLOCK output pin high to block other cores |
|`000111`| `AS.16` | `*($rd + i8).low <= $rs.low` (2), but set the BLOCK output pin high to block other cores |
|`001000`| `AL.8` | `$rd.low.low <= *($rs + i8).low.low` (2), but set the BLOCK output pin high to block other cores |
|`001001`| `AS.8` | `*($rd + i8).low.low <= $rs.low.low` (2), but set the BLOCK output pin high to block other cores |
|`001010`| `SSEG` | `segments($rd) <= $rs, $rs2`, nop in usermode |
|`001011`| `GSEG` | `$rd, $rs2 <= segments($rd)`, nop in usermode |
|`001100`| `BFAR` | `$pr, $cs <= i19, $rs` nop in usermode |
|`001101`| `SYSENTER` | `r30, r31 <= $pr, $cs, <= $pr, $cs <= 16, 0, $usr <= 0` trashes r30, r31 |
|`001110`| `SYSEXIT` | `$pr, $cs <= r30, r31, $usr <= 1` nop in usermode |

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
| `31:13` | `IF EXTEND (rs2, subop, i8) ELSEIF NOT ALU (i19) ELSE (rs2, i14)` |

## note
- in Bcc, we use $rd as the i4, so we can have i23 aswell
- fixed 32 bit opcode length

# Memory
- memory is 8 bits wide, with up to 2^32 elements

# Notes
- branch immediates are sign extended
- i23,i4 in branch,memory access is sign extended
- the carry for ADD,SUB,XOR,ORR,AND is always the carry of the ADD/SUB node.

# Implementation
- TODO
