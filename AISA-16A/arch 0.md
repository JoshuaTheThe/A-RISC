# Very simple 16 bit architecture
| MNemonic | Description | width (bits)
|---|---|---|
| `$rr` | Result Register | 16 |
| `$xr` | Index Register | 16 |
| `$lr` | Link Register | 16 |
| `$pr` | Program Register | 16 |
| `$be` | Result carried or is zero (bigger or equal to) | 1 |

# Opcode
| Code | MNemonic Prefix | Operation
|---|---|---|
| `00000` | `BRI` | `$lr ← $pr, $pr ← $pr + imm11` |
| `00001` | `BBE` | `IF $be THEN $lr ← $pr, $pr ← $pr + imm11 ELSE $pr ← $pr` |
| `00010` | `BAR` | `$pr ← $rr` |
| `00011` | `NOP` | `$pr ← $pr` |
| `00100` | `NOP` | `$pr ← $pr` |
| `00101` | `NOP` | `$pr ← $pr` |
| `00110` | `NOP` | `$pr ← $pr` |
| `00111` | `NOP` | `$pr ← $pr` |
| `01000` | `ADD` | `$rr ← $rr + *(imm11 + $xr)` |
| `01001` | `SUB` | `$rr ← $rr - *(imm11 + $xr)` |
| `01010` | `XRR` | `$rr ← $rr ^ *(imm11 + $xr)` |
| `01011` | `ORR` | `$rr ← $rr | *(imm11 + $xr)` |
| `01100` | `AND` | `$rr ← $rr & *(imm11 + $xr)` |
| `01101` | `NOP` | `$rr ← $rr` |
| `01110` | `NOP` | `$rr ← $rr` |
| `01111` | `NOP` | `$rr ← $rr` |
| `10000` | `LRI` | `$rr ← imm11` |
| `10001` | `LMX` | `$rr ← *(imm11 + $xr)` |
| `10010` | `SMX` | `*(imm11 + $xr) ← $rr` |
| `10011` | `NOP` | `$xr ← $xr` |
| `10100` | `LXR` | `$xr ← $rr` |
| `10101` | `LRX` | `$rr ← $xr` |
| `10110` | `INX` | `$xr ← $xr + 1` |
| `10111` | `DEX` | `$xr ← $xr - 1` |
| `11000` | `RET` | `$pr ← $lr` |
| `11001` | `LRL` | `$rr ← $lr` |
| `11010` | `LLR` | `$lr ← $rr` |
| `11011` | `LMR` | `$rr ← *imm11` |
| `11100` | `SMR` | `*imm11 ← $rr` |

# opcode encoding
| bits | desc |
|------|------|
| `4:0` | opcode |
| `15:5` | imm11 |

# Memory
- memory is 16 bits wide, with 65536 elements

```asm
        ; Use XR as stack pointer
        ; If you need it for something, remember to restore it
        ; Save return address
        ; All literals are hex

        ;   destination is in memory location $0000
        ;   source is in memory location $0002
        ;   length is in memory location $0004
        ;   temp is in memory location $0008
        ;   temp2 is in memory location $000A
MEMCPY: DEX     ; Push return address
        LRL
        SMX     $0000
        LRX             ; rr = xr
        SMR     $000A   ; save Stack poiner
LOOP:   LMR     $0002   ; rr = source
        LXR             ; xr = rr
        LMX     $0000   ; rr = *(xr + 0)
        SMR     $0008   ; temp = rr
        INX             ; ++xr
        LRX             ; rr = xr
        SMR     $0002   ; source = xr
        LMR     $0000   ; rr = dest
        LXR             ; xr = rr
        LMR     $0008   ; rr = temp
        SMX     $0000   ; *(xr + 0) = rr
        INX             ; ++xr
        LRX             ; rr = xr
        SMR     $0000   ; dest = rr
        LMR     $0004   ; rr = len
        LXR             ; xr = rr
        DEX             ; --xr
        LRX             ; rr = xr
        SMR     $0004   ; len = rr
        BBE     END     ; exit if <= 0
        BRI     LOOP    ; continue
END:    LMR     $000A   ; restore Stack pointer
        LXR
        LMX     $0000   ; Pop return address
        LLR
        INX
        RET
START:  LRI     $FFFF   ; top of stack
        LXR
        ; arguments go here
        LRI     $DEST
        SMR     $0000
        LRI     $SRC
        SMR     $0002
        LMR     $LEN
        SMR     $0004
        BRI     MEMCPY
        BRI     $
DEST:   .WORDS  0, 0, 0, 0
SRC:    .WORDS  1, 1, 1, 1
LEN:    .WORD   4
```

# Notes
- the intended implementation is inside a Game, (scrapmechanic), therefore, the upper **6** bits of memory access is ignored for the main implementation, but, you CAN use the whole address for your impl
