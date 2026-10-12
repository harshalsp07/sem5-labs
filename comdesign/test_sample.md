# Test Sample: Hex Data for 8085sim

## Test Program (Mini-C Source)

```c
int a;
int b = 5;
int c = a * b + 3;
int d;
d = c - b * 2;
```

This tests: declarations (with and without initializers), assignment statements,
all four arithmetic operators (`+`, `-`, `*`, `/`), and variable references.

---

## Source Code as ASCII Hex (Load at 2000H)

Copy these hex bytes into memory starting at address **2000H**:

```
Address : Hex Bytes                                        : ASCII
--------:--------------------------------------------------:-----------------
2000    : 69 6E 74 20 61 3B 0A                             : int a;\n
2007    : 69 6E 74 20 62 20 3D 20 35 3B 0A                 : int b = 5;\n
2012    : 69 6E 74 20 63 20 3D 20 61 20 2A 20 62 20 2B 20  : int c = a * b + 
2022    : 33 3B 0A                                         : 3;\n
2025    : 69 6E 74 20 64 3B 0A                             : int d;\n
202C    : 64 20 3D 20 63 20 2D 20 62 20 2A 20 32 3B 0A    : d = c - b * 2;\n
203B    : 00                                               : NULL (terminator)
```

### Flat Hex Dump (60 bytes total)

```
2000: 69 6E 74 20 61 3B 0A 69 6E 74 20 62 20 3D 20 35
2010: 3B 0A 69 6E 74 20 63 20 3D 20 61 20 2A 20 62 20
2020: 2B 20 33 3B 0A 69 6E 74 20 64 3B 0A 64 20 3D 20
2030: 63 20 2D 20 62 20 2A 20 32 3B 0A 00
```

---

## Expected Output After Compilation (at 2800H)

After running the compiler (PC=4000H), the generated code at 2800H should be:

```
; === Preamble (34 bytes) ===
2800: C3 22 28                 ; JMP 2822H (skip to compiled code)
; --- MUL8 subroutine ---
2803: 4F 78 B7 CA 12 28        ; MOV C,A; MOV A,B; ORA A; JZ 2812H
2809: 57 3E 00                 ; MOV D,A; MVI A,00H
280C: 81 15 C2 0C 28           ; ADD C; DCR D; JNZ 280CH
2811: C9                       ; RET
2812: 3E 00 C9                 ; MVI A,00H; RET
; --- DIV8 subroutine ---
2815: 0E 00                    ; MVI C,00H
2817: B8 DA 20 28              ; CMP B; JC 2820H
281B: 90 0C C3 17 28           ; SUB B; INR C; JMP 2817H
2820: 79 C9                    ; MOV A,C; RET

; === Compiled Code ===
; Line: int a;  --> (no code, just symbol table entry)

; Line: int b = 5;
2822: 3E 05                    ; MVI A, 05H
2824: 32 01 38                 ; STA 3801H

; Line: int c = a * b + 3;
2827: 3A 00 38                 ; LDA 3800H        (load a)
282A: F5                       ; PUSH PSW          (save a)
282B: 3A 01 38                 ; LDA 3801H        (load b)
282E: 47                       ; MOV B, A          (B = b)
282F: F1                       ; POP PSW           (A = a)
2830: CD 03 28                 ; CALL 2803H        (A = a * b)
2833: F5                       ; PUSH PSW          (save a*b)
2834: 3E 03                    ; MVI A, 03H        (A = 3)
2836: 47                       ; MOV B, A          (B = 3)
2837: F1                       ; POP PSW           (A = a*b)
2838: 80                       ; ADD B              (A = a*b + 3)
2839: 32 02 38                 ; STA 3802H         (store c)

; Line: int d;  --> (no code)

; Line: d = c - b * 2;
283C: 3A 02 38                 ; LDA 3802H        (load c)
283F: F5                       ; PUSH PSW          (save c)
2840: 3A 01 38                 ; LDA 3801H        (load b)
2843: F5                       ; PUSH PSW          (save b)
2844: 3E 02                    ; MVI A, 02H        (A = 2)
2846: 47                       ; MOV B, A          (B = 2)
2847: F1                       ; POP PSW           (A = b)
2848: CD 03 28                 ; CALL 2803H        (A = b * 2)
284B: 47                       ; MOV B, A          (B = b*2)
284C: F1                       ; POP PSW           (A = c)
284D: 90                       ; SUB B              (A = c - b*2)
284E: 32 03 38                 ; STA 3803H         (store d)

; End
2851: 76                       ; HLT
```

---

## Expected Variable Values After Running Generated Code (PC=2800H)

| Address | Variable | Expected Value | Calculation |
|---------|----------|---------------|-------------|
| `3800H` | `a` | `00H` (0)   | Uninitialized |
| `3801H` | `b` | `05H` (5)   | Initialized to 5 |
| `3802H` | `c` | `03H` (3)   | a*b + 3 = 0*5 + 3 = 3 |
| `3803H` | `d` | `F9H` (-7)  | c - b*2 = 3 - 10 = -7 (two's complement: F9H) |

---

## Additional Test Programs

### Test 2: Simple Addition and Subtraction

```c
int x = 10;
int y = 3;
int z = x + y - 2;
```

**Hex at 2000H:**
```
2000: 69 6E 74 20 78 20 3D 20 31 30 3B 0A
200C: 69 6E 74 20 79 20 3D 20 33 3B 0A
2018: 69 6E 74 20 7A 20 3D 20 78 20 2B 20 79 20 2D 20
2028: 32 3B 0A 00
```

**Expected results:**
| Address | Variable | Value |
|---------|----------|-------|
| `3800H` | `x` | `0AH` (10) |
| `3801H` | `y` | `03H` (3) |
| `3802H` | `z` | `0BH` (11) → 10 + 3 - 2 |

---

### Test 3: Division

```c
int p = 20;
int q = 4;
int r = p / q;
```

**Hex at 2000H:**
```
2000: 69 6E 74 20 70 20 3D 20 32 30 3B 0A
200C: 69 6E 74 20 71 20 3D 20 34 3B 0A
2018: 69 6E 74 20 72 20 3D 20 70 20 2F 20 71 3B 0A 00
```

**Expected results:**
| Address | Variable | Value |
|---------|----------|-------|
| `3800H` | `p` | `14H` (20) |
| `3801H` | `q` | `04H` (4) |
| `3802H` | `r` | `05H` (5) → 20 / 4 |

---

### Test 4: Reassignment

```c
int a = 10;
int b = 20;
a = a + b;
b = a - b;
```

**Hex at 2000H:**
```
2000: 69 6E 74 20 61 20 3D 20 31 30 3B 0A
200C: 69 6E 74 20 62 20 3D 20 32 30 3B 0A
2018: 61 20 3D 20 61 20 2B 20 62 3B 0A
2023: 62 20 3D 20 61 20 2D 20 62 3B 0A 00
```

**Expected results (after running generated code):**
| Address | Variable | Value | Explanation |
|---------|----------|-------|-------------|
| `3800H` | `a` | `1EH` (30) | 10 + 20 = 30 |
| `3801H` | `b` | `0AH` (10) | 30 - 20 = 10 |

---

## Quick Reference: ASCII Character Codes

| Char | Hex | | Char | Hex | | Char | Hex |
|------|-----|-|------|-----|-|------|-----|
| space | 20 | | `+` | 2B | | `a` | 61 |
| `*` | 2A | | `-` | 2D | | `b` | 62 |
| `/` | 2F | | `0` | 30 | | `c` | 63 |
| `;` | 3B | | `1` | 31 | | `d` | 64 |
| `=` | 3D | | `5` | 35 | | `i` | 69 |
| `\n` | 0A | | `9` | 39 | | `n` | 6E |
| `\0` | 00 | |     |     | | `t` | 74 |
