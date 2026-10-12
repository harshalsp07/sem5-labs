# Step-by-Step Guide: MiniC-to-8085 Compiler

## What This Is

A **self-hosted compiler** written entirely in 8085 assembly language. It reads a miniature C program stored as ASCII text in memory and produces 8085 machine code (hex bytes) in another memory region. The generated code can then be executed directly on the 8085.

---

## Step 1: Memory Map

The compiler divides memory into distinct regions:

| Address Range | Size | Purpose |
|---------------|------|---------|
| `2000H–27FFH` | 2 KB | **Source Code** — Your mini-C program as ASCII text, null-terminated |
| `2800H–2FFFH` | 2 KB | **Generated Code** — The 8085 machine code output |
| `3000H–37FFH` | 2 KB | **Symbol Table** — Maps variable names to memory addresses |
| `3800H–3FFFH` | 2 KB | **Variable Storage** — Runtime storage for declared variables |
| `4000H+`      | —    | **Compiler Code** — The compiler program itself |
| `FFFFH ↓`     | —    | **Stack** — Grows downward from top of memory |

---

## Step 2: How the Compiler Pipeline Works

The compiler runs in a single pass through three tightly integrated phases:

### 2a. Lexer (Tokenizer)

The lexer (`GET_TK`) scans the ASCII source code character by character and produces **tokens**:

| Token Type | Code | Example |
|------------|------|---------|
| `TOK_EOF`  | 00H  | End of source (null byte) |
| `TOK_INT`  | 01H  | The keyword `int` |
| `TOK_IDENT`| 02H  | Variable name like `a`, `count` |
| `TOK_NUM`  | 03H  | Number like `5`, `42`, `255` |
| `TOK_PLUS` | 04H  | `+` |
| `TOK_MINUS`| 05H  | `-` |
| `TOK_STAR` | 06H  | `*` |
| `TOK_SLASH`| 07H  | `/` |
| `TOK_EQUAL`| 08H  | `=` |
| `TOK_SEMI` | 09H  | `;` |

**How it works:**
1. Skip whitespace (spaces, tabs, CR, LF)
2. If null byte → `TOK_EOF`
3. If letter → read alphanumeric sequence → check if `"int"` → `TOK_INT` or `TOK_IDENT`
4. If digit → read all digits, convert ASCII to binary (multiply-by-10 accumulation) → `TOK_NUM`
5. If operator/punctuation → return corresponding single-char token

### 2b. Parser (Recursive Descent)

The parser follows the grammar using recursive-descent parsing. Each grammar rule maps to an assembly subroutine:

```
PARSE_PG  → program      → decl_or_stmt*
PARSE_DC  → decl         → 'int' IDENT ('=' expr)?
PARSE_ST  → stmt         → IDENT '=' expr
PARSE_EX  → expr         → term (('+' | '-') term)*
PARSE_TM  → term         → factor (('*' | '/') factor)*
PARSE_FC  → factor       → IDENT | NUMBER
```

**Operator precedence** is enforced structurally:
- `*` and `/` are handled in `PARSE_TM` (tighter binding)
- `+` and `-` are handled in `PARSE_EX` (looser binding)

This means `a + b * c` is correctly parsed as `a + (b * c)`.

### 2c. Code Generator

Code is generated **during parsing** — each parser routine emits 8085 opcodes as it recognizes constructs:

| Mini-C Construct | Generated 8085 Code |
|-----------------|---------------------|
| Number `5` | `MVI A, 05H` (3E 05) |
| Variable `a` | `LDA addr_a` (3A xx xx) |
| `x = expr` | `STA addr_x` (32 xx xx) |
| `a + b` | `PUSH PSW; <b>; MOV B,A; POP PSW; ADD B` |
| `a - b` | `PUSH PSW; <b>; MOV B,A; POP PSW; SUB B` |
| `a * b` | `PUSH PSW; <b>; MOV B,A; POP PSW; CALL MUL8` |
| `a / b` | `PUSH PSW; <b>; MOV B,A; POP PSW; CALL DIV8` |

**Expression evaluation strategy:** The first operand's result is pushed onto the stack (`PUSH PSW`), the second operand is computed, moved to register B, the first operand is popped back to A (`POP PSW`), and then the operation is performed. This handles arbitrary nesting correctly.

---

## Step 3: Symbol Table

Each variable gets an 11-byte entry:

```
Bytes 0-8:  Variable name (up to 8 chars + null terminator)
Bytes 9-10: Memory address (low byte, high byte) in 3800H+ region
```

- First variable declared → address `3800H`
- Second variable → `3801H`
- Third → `3802H`, and so on...

---

## Step 4: Generated Code Structure

The output at `2800H` has this layout:

```
2800H: C3 22 28        JMP 2822H     ; Skip helper routines
2803H: [MUL8 routine]                 ; 18 bytes, multiply A*B
2815H: [DIV8 routine]                 ; 13 bytes, divide A/B
2822H: [compiled code]                ; Your program's machine code
  ...
????H: 76              HLT           ; End of program
```

The **MUL8** routine (at 2803H) multiplies A × B via repeated addition.
The **DIV8** routine (at 2815H) divides A ÷ B via repeated subtraction.

---

## Step 5: How to Use with 8085sim

### Loading the Compiler

1. Open your 8085 simulator
2. Load `MiniC_Compiler.asm` or assemble it and load the hex at address `4000H`

### Loading the Source Code

3. Enter your mini-C program as ASCII bytes starting at address `2000H`
4. Terminate with a `00H` byte (null terminator)

**Example:** To enter `int a = 5;` at 2000H:
```
2000: 69 6E 74 20 61 20 3D 20 35 3B 00
       i  n  t  sp a  sp =  sp 5  ;  NULL
```

### Running the Compiler

5. Set Program Counter (PC) to `4000H`
6. Run the program
7. The compiler will process the source and write machine code to `2800H+`
8. The compiler halts when done

### Running the Compiled Program

9. Set PC to `2800H`
10. Run the program
11. The compiled program executes and halts

### Checking Results

12. Examine memory starting at `3800H` to see variable values:
    - `3800H` = first declared variable
    - `3801H` = second declared variable
    - etc.

---

## Step 6: Compilation Walkthrough

Let's trace through the example program:

```c
int a;
int b = 5;
int c = a * b + 3;
int d;
d = c - b * 2;
```

### Symbol Table After Compilation

| Variable | Symbol Table Entry | Storage Address |
|----------|-------------------|-----------------|
| `a` | 3000H: `61 00 ... 00 38` | `3800H` |
| `b` | 300BH: `62 00 ... 01 38` | `3801H` |
| `c` | 3016H: `63 00 ... 02 38` | `3802H` |
| `d` | 3021H: `64 00 ... 03 38` | `3803H` |

### Generated Code at 2822H

**Line 1: `int a;`** → No code generated (just added to symbol table)

**Line 2: `int b = 5;`**
```
2822: 3E 05        MVI A, 05H       ; A = 5
2824: 32 01 38     STA 3801H        ; b = 5
```

**Line 3: `int c = a * b + 3;`**
```
2827: 3A 00 38     LDA 3800H        ; A = a
282A: F5           PUSH PSW          ; save a
282B: 3A 01 38     LDA 3801H        ; A = b
282E: 47           MOV B, A          ; B = b
282F: F1           POP PSW           ; A = a
2830: CD 03 28     CALL 2803H        ; A = a * b (MUL8)
2833: F5           PUSH PSW          ; save a*b
2834: 3E 03        MVI A, 03H       ; A = 3
2836: 47           MOV B, A          ; B = 3
2837: F1           POP PSW           ; A = a*b
2838: 80           ADD B             ; A = a*b + 3
2839: 32 02 38     STA 3802H        ; c = a*b + 3
```

**Line 4: `int d;`** → No code generated

**Line 5: `d = c - b * 2;`**
```
283C: 3A 02 38     LDA 3802H        ; A = c
283F: F5           PUSH PSW          ; save c
2840: 3A 01 38     LDA 3801H        ; A = b
2843: F5           PUSH PSW          ; save b
2844: 3E 02        MVI A, 02H       ; A = 2
2846: 47           MOV B, A          ; B = 2
2847: F1           POP PSW           ; A = b
2848: CD 03 28     CALL 2803H        ; A = b * 2 (MUL8)
284B: 47           MOV B, A          ; B = b*2
284C: F1           POP PSW           ; A = c
284D: 90           SUB B             ; A = c - b*2
284E: 32 03 38     STA 3803H        ; d = c - b*2
```

**End:**
```
2851: 76           HLT              ; Stop
```

### Runtime Results

Assuming memory is initialized to 0 before running:

| Variable | Address | Value | Explanation |
|----------|---------|-------|-------------|
| `a` | `3800H` | `00H` (0) | Uninitialized (default 0) |
| `b` | `3801H` | `05H` (5) | Initialized to 5 |
| `c` | `3802H` | `03H` (3) | 0 * 5 + 3 = 3 |
| `d` | `3803H` | `F9H` (-7) | 3 - 5*2 = 3 - 10 = -7 (two's complement) |

---

## Limitations

1. **8-bit values only**: Variables hold 0–255 (unsigned) or -128 to 127 (signed). Overflow wraps around.
2. **No error handling**: The compiler assumes valid input. Malformed code will produce garbage.
3. **Max 8-character identifiers**: Variable names are truncated to 8 characters.
4. **No parentheses**: Expressions rely on `*`/`/` binding tighter than `+`/`-`.
5. **No unary minus**: Use `0 - x` instead of `-x`.
6. **~20 variables max**: Limited by symbol table size (2 KB / 11 bytes per entry ≈ 186, but practical limit is lower).
