# MiniC-to-8085 Compiler & Lexer — Task Prompt

I want to build a self-hosted **MiniC-to-8085 Compiler** and a standalone **Lexical Analyzer (Scanner)** in both **Intel 8085 Assembly** and **Flex**, along with comprehensive documentation and a step-by-step guide for the **Sim8085 website** (`sim8085.com`).

---

### 1. MiniC Language & Grammar Specification

The input language is a miniature subset of C supporting integer variables, multi-digit numbers ($0 \dots 255$), arithmetic operators, assignments, semicolons, and single-line comments (`#` and `//`).

It must adhere to the following grammar:

```
program      → decl_or_stmt*
decl_or_stmt → decl ';' | stmt ';'
decl         → 'int' IDENT ('=' expr)?
stmt         → IDENT '=' expr
expr         → term (('+' | '-') term)*
term         → factor (('*' | '/') factor)*
factor       → IDENT | NUMBER
```

#### Reference Test Program:
```c
int a;
int b = 5;
int c = a * b + 3;
int d;
d = c - b * 2;
```

---

### 2. Core Requirements

#### A. Full 8085 MiniC Compiler (`MiniC_Compiler.asm`)
1. Written **entirely in Intel 8085 Assembly** (self-hosted).
2. Reads the input MiniC program as ASCII text from a fixed memory location (**`2000H`**, null-terminated).
3. Parses declarations, assignments, and expressions with correct operator precedence (`*`/`/` before `+`/`-`).
4. Generates 8085 machine code directly into memory starting at **`2800H`** (with embedded 8-bit `MUL8` and `DIV8` helper subroutines).
5. Allocates runtime variable storage starting at **`3800H`**.
6. When executed starting at `2800H`, it computes the expected results in variable memory.

---

#### B. Standalone Lexer in `Lexer/` Folder
Create a clean, minimal `Lexer/` directory containing only the essential files:

1. **`lexer.asm` (Pure 8085 Assembly Lexer)**:
   - Scans MiniC ASCII source code starting at memory **`2000H`**.
   - Converts lexemes into structured **2-byte token records** `(Token_Type, Token_Value)` and writes them sequentially into memory starting at **`2800H`**.
   - Token mapping:
     - `00H` = `TOK_EOF`
     - `01H` = `TOK_INT` (`int`)
     - `02H` = `TOK_IDENT` (Variable name, stores ASCII character code)
     - `03H` = `TOK_NUM` (Numeric constant 0..255, stores 8-bit binary value)
     - `04H` = `TOK_PLUS` (`+`)
     - `05H` = `TOK_MINUS` (`-`)
     - `06H` = `TOK_STAR` (`*`)
     - `07H` = `TOK_SLASH` (`/`)
     - `08H` = `TOK_EQUAL` (`=`)
     - `09H` = `TOK_SEMI` (`;`)
   - Uses shift-and-add arithmetic ($8E + 2E + d$) to accumulate multi-digit decimal numbers without a hardware multiplier.
   - Skips whitespace and single-line comments (`#` and `//`).
   - Tracks state in memory at `7F00H`: `SRC_PTR` (`7F00H`), `TOK_PTR` (`7F02H`), `TOK_CNT` (`7F04H`), and `LINE_NUM` (`7F05H`).

2. **`lexer.l` (Flex / Lex Specification)**:
   - Declarative scanner using regular expressions matching all MiniC tokens.
   - Outputs a clean, formatted token table and a simulated 8085 memory dump at `2800H` matching the assembly output byte-for-byte.

3. **`REPORT.md` (Comprehensive Technical Lab Report)**:
   - In-depth laboratory report explaining:
     - MiniC formal lexical specification and token encoding scheme.
     - Complete Finite State Automaton (DFA) state diagram (Mermaid) and transition matrix.
     - 8085 memory architecture map and register allocation strategy.
     - Subroutine breakdown (`GET_NEXT_TOKEN`, `MULT10`, `IS_LETTER`, `IS_DIGIT`, `CHECK_KEYWORD_INT`).
     - Step-by-step trace tables for the reference program with exact memory dumps before (`2000H`) and after (`2800H`).
     - Comparative analysis between bare-metal 8085 Assembly and modern Flex.

4. **`SIM8085.md` (Sim8085 Web Simulator Guide)**:
   - Clear, step-by-step instructions on how to load, assemble, enter hex input at `2000H`, execute, and showcase results on the **Sim8085 website** (`https://sim8085.com`).
   - Includes exact ASCII hex memory input blocks, expected token memory dumps at `2800H`, status variable addresses at `7F00H`, and multiple demonstration test suites.

5. **`Makefile`**:
   - Clean build file to compile and test the Flex lexer with `make test` and clean build artifacts with `make clean`.
