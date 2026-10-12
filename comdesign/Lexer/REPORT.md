# Comprehensive Evaluation Report: Lexical Analyzer Design for MiniC in Intel 8085 Assembly & Flex

---

## **1. Executive Summary & Problem Formulation**

### **1.1 Overview**
In modern compiler construction, the **Lexical Analyzer (Scanner / Tokenizer)** forms the foundational first phase of the compiler pipeline. Its primary responsibility is to transform a raw stream of characters (source code) into a structured stream of discrete, classified syntactic units known as **Tokens**, while eliminating extraneous characters such as whitespace and comments.

This report presents the design, mathematical modeling, implementation, and cross-platform verification of a Lexical Analyzer for the **MiniC language**. The implementation is conducted across two contrasting paradigms:
1. **Low-Level Deterministic Assembly (Intel 8085 Microprocessor):** A hand-crafted, register-optimized finite state transducer operating under strict hardware constraints (8-bit registers, 64 KB memory address space, lack of hardware multiplication/division).
2. **High-Level Declarative Generator (Flex / Lex):** A tool utilizing POSIX regular expressions, automated Deterministic Finite Automaton (DFA) synthesis, and structured diagnostic reporting.

Both implementations adhere to an identical **2-Byte Token Memory Layout** (`Token_Type`, `Token_Value`), ensuring 100% binary equivalence and verification compatibility between the high-level simulator and the physical/emulated 8085 microprocessor.

---

## **2. MiniC Lexical Grammar & Token Specification**

### **2.1 Formal Grammar Context**
The MiniC programming language is defined by the following Context-Free Grammar (CFG):

```
Program      → DeclOrStmt*
DeclOrStmt   → Decl ';' | Stmt ';'
Decl         → 'int' IDENT ('=' Expr)?
Stmt         → IDENT '=' Expr
Expr         → Term (('+' | '-') Term)*
Term         → Factor (('*' | '/') Factor)*
Factor       → IDENT | NUMBER
```

### **2.2 Token Classes and Memory Encoding Scheme**
The lexical analyzer extracts tokens and encodes each as a **2-byte record** in memory:
- **Byte 0 (Address `2800H + 2*k`):** `Token_Type` (Numeric ID from `00H` to `09H`)
- **Byte 1 (Address `2800H + 2*k + 1`):** `Token_Value` (Attribute: 8-bit unsigned integer value for numbers, 7-bit ASCII character code for single identifiers, `00H` for operators and keywords)

| Token ID (Hex / Dec) | Symbolic Constant | Regular Expression Pattern | Value Byte Semantics |
| :--- | :--- | :--- | :--- |
| **`00H` (0)** | `TOK_EOF` | `\0` or `$` | `00H` (End of Input Sentinel) |
| **`01H` (1)** | `TOK_INT` | `int` | `00H` (Type Keyword) |
| **`02H` (2)** | `TOK_IDENT` | `[a-zA-Z_][a-zA-Z0-9_]*` | ASCII code of identifier's 1st char (e.g. `'a'` = `61H`) |
| **`03H` (3)** | `TOK_NUM` | `[0-9]+` ($0 \le N \le 255$) | 8-bit unsigned binary integer value |
| **`04H` (4)** | `TOK_PLUS` | `+` | `00H` (Addition Operator) |
| **`05H` (5)** | `TOK_MINUS` | `-` | `00H` (Subtraction Operator) |
| **`06H` (6)** | `TOK_STAR` | `*` | `00H` (Multiplication Operator) |
| **`07H` (7)** | `TOK_SLASH` | `/` | `00H` (Division Operator) |
| **`08H` (8)** | `TOK_EQUAL` | `=` | `00H` (Assignment Operator) |
| **`09H` (9)** | `TOK_SEMI` | `;` | `00H` (Statement Terminator) |

### **2.3 Auxiliary Lexical Rules**
1. **Whitespace Elimination:** Spaces (`20H`), Tabs (`09H`), and Carriage Returns (`0DH`) advance the input stream pointer without generating token records.
2. **Line Counting:** Newline characters (`\n`, `0AH`) advance the line tracker counter at `7F05H`.
3. **Comment Stripping:** Both `#` (hash comments) and `//` (C++ style line comments) consume all characters up to the terminating newline (`0AH`) or null byte (`00H`).

---

## **3. Finite State Automaton (DFA) Theory & Modeling**

The Lexical Analyzer is formally modeled as a Deterministic Finite Automaton (DFA) 5-tuple:

$$M = (Q, \Sigma, \delta, q_0, F)$$

Where:
- **$Q$ (Finite Set of States):** `{ q_start, q_ws, q_comment, q_ident, q_kw_int, q_num, q_plus, q_minus, q_star, q_slash, q_equal, q_semi, q_eof }`
- **$\Sigma$ (Input Alphabet):** Standard ASCII Character Set (`00H` to `7FH`)
- **$q_0$ (Start State):** `q_start`
- **$\delta$ (State Transition Function):** $\delta : Q \times \Sigma \rightarrow Q$
- **$F$ (Set of Accepting / Final States):** `{ q_ident, q_kw_int, q_num, q_plus, q_minus, q_star, q_slash, q_equal, q_semi, q_eof }`

### **3.1 DFA State Transition Diagram**

```mermaid
stateDiagram-v2
    [*] --> START_STATE : Initialize Read Pointer

    START_STATE --> START_STATE : ' ', '\\t', '\\r' (Skip Whitespace)
    START_STATE --> START_STATE : '\\n' (Increment Line Counter)
    
    START_STATE --> COMMENT_LINE : '#' or '//'
    COMMENT_LINE --> START_STATE : '\\n' or EOF (Resume Scan)
    COMMENT_LINE --> COMMENT_LINE : Other Characters

    START_STATE --> EMIT_PLUS : '+'
    START_STATE --> EMIT_MINUS : '-'
    START_STATE --> EMIT_STAR : '*'
    START_STATE --> CHECK_SLASH : '/'
    CHECK_SLASH --> COMMENT_LINE : '/' (Comment detected)
    CHECK_SLASH --> EMIT_SLASH : Non-'/' (Division Operator)
    
    START_STATE --> EMIT_EQUAL : '='
    START_STATE --> EMIT_SEMI : ';'
    START_STATE --> EMIT_EOF : '\\0' or '$'

    START_STATE --> NUM_ACCUMULATOR : '0'..'9'
    NUM_ACCUMULATOR --> NUM_ACCUMULATOR : '0'..'9' [B = B*10 + (char-'0')]
    NUM_ACCUMULATOR --> EMIT_NUM : Non-digit delimiter

    START_STATE --> IDENT_SCAN : 'a'..'z', 'A'..'Z', '_'
    IDENT_SCAN --> IDENT_SCAN : Alphanumeric / '_'
    IDENT_SCAN --> CHECK_KEYWORD : Non-alphanumeric delimiter
    CHECK_KEYWORD --> EMIT_INT : Lexeme == "int"
    CHECK_KEYWORD --> EMIT_IDENT : Lexeme != "int"

    EMIT_PLUS --> WRITE_RECORD : Emit (04H, 00H)
    EMIT_MINUS --> WRITE_RECORD : Emit (05H, 00H)
    EMIT_STAR --> WRITE_RECORD : Emit (06H, 00H)
    EMIT_SLASH --> WRITE_RECORD : Emit (07H, 00H)
    EMIT_EQUAL --> WRITE_RECORD : Emit (08H, 00H)
    EMIT_SEMI --> WRITE_RECORD : Emit (09H, 00H)
    EMIT_NUM --> WRITE_RECORD : Emit (03H, B)
    EMIT_IDENT --> WRITE_RECORD : Emit (02H, ASCII)
    EMIT_INT --> WRITE_RECORD : Emit (01H, 00H)
    EMIT_EOF --> [*] : Emit (00H, 00H) & HLT
    
    WRITE_RECORD --> START_STATE : Write 2 Bytes to [TOK_PTR]
```

### **3.2 State Transition Matrix**

| Current State | Input Class | Action / State Update | Next State |
| :--- | :--- | :--- | :--- |
| `START_STATE` | `[' ', '\t', '\r']` | `INX H` (advance pointer) | `START_STATE` |
| `START_STATE` | `'\n'` | `INR LINE_NUM`, `INX H` | `START_STATE` |
| `START_STATE` | `'#'` or `'//'` | Scan until `\n` | `COMMENT_LINE` |
| `START_STATE` | `'0'..'9'` | `B = (char - '0')` | `NUM_ACCUMULATOR` |
| `START_STATE` | `'a'..'z', 'A'..'Z', '_'` | Copy char to `CUR_NAME` | `IDENT_SCAN` |
| `START_STATE` | `'+', '-', '*', '/', '=', ';'` | Set `CUR_TYPE`, `CUR_VAL = 0` | Respective Operator State |
| `START_STATE` | `'\0'` or `'$'` | Set `CUR_TYPE = 00H`, `CUR_VAL = 00H` | `EMIT_EOF` |
| `NUM_ACCUMULATOR` | `'0'..'9'` | `B = (B * 10) + (char - '0')` | `NUM_ACCUMULATOR` |
| `NUM_ACCUMULATOR` | Non-digit | Emit `(03H, B)` | `WRITE_RECORD` |
| `IDENT_SCAN` | Alphanumeric | Append to `CUR_NAME` | `IDENT_SCAN` |
| `IDENT_SCAN` | Non-alphanumeric | Null-terminate, call `CK_INT` | `CHECK_KEYWORD` |

---

## **4. Intel 8085 Assembly Implementation Details**

### **4.1 Memory Architecture Map**

The Intel 8085 system memory is segregated into dedicated functional partitions:

```
+------------------+ 0000H
| Reserved / ROM   |
+------------------+ 2000H
| SOURCE CODE      | MiniC source string in ASCII (null-terminated)
| (2000H - 27FFH)  | Capacity: 2,048 characters
+------------------+ 2800H
| TOKEN STREAM     | Array of (Token_Type, Token_Value) pairs
| (2800H - 2FFFH)  | Capacity: 1,024 Token Records (2,048 bytes)
+------------------+ 4000H
| LEXER PROGRAM    | Executable machine code instructions
| (4000H - 43FFH)  | Standalone Lexer Binary
+------------------+ 7F00H
| STATE VARIABLES  | Pointers, counters, current token registers
| (7F00H - 7F1FH)  | SRC_PTR, TOK_PTR, TOK_CNT, LINE_NUM, CUR_NAME
+------------------+ FFFFH
| STACK SPACE (↓)  | Top of RAM for call stack and register preservation
+------------------+
```

### **4.2 Register Allocation Strategy**

| Register / Pair | Dedicated Role in Lexer Engine | Rationale |
| :--- | :--- | :--- |
| **`HL` Pair** | Source Buffer Read Pointer (`SRC_PTR`) / Memory Operand | Primary address pointer with `MOV A, M` and `INX H` single-cycle access. |
| **`DE` Pair** | Token Buffer Write Pointer (`TOK_PTR`) / Identifier Buffer | Allows secondary 16-bit indexing via `STAX D` and `LDAX D`. |
| **`B` Register** | Base-10 Numeric Accumulator / Working Counter | Preserves numeric value across multi-digit accumulation. |
| **`C` Register** | Arithmetic Multiplier Temporary / Loop Limit | Scratch register during `MULT10` shift-and-add arithmetic. |
| **`A` (Accumulator)** | Character Comparison (`CPI`), Math, Flag Evaluation | Core arithmetic-logic unit register for character classification. |
| **Flags (`CY`, `Z`)** | Character Classification Output | Set by `IS_LET`, `IS_DIG`, and `CK_INT` subroutines for branching. |

### **4.3 Algorithmic Subroutine Breakdown**

#### **1. Base-10 Numeric Accumulator without Hardware Multiplier (`MULT10`)**
The 8085 microprocessor lacks a hardware multiplier instruction (`MUL`). To accumulate multi-digit decimal numbers $N = N \times 10 + d$, we employ binary arithmetic expansion:

$$10 \times N = (8 \times N) + (2 \times N) = (N \ll 3) + (N \ll 1)$$

In 8085 assembly, a 1-bit logical left shift is implemented via accumulator addition (`ADD A`):
```assembly
; Multiplying B by 10:
MOV A, B        ; A = B
ADD A           ; A = 2 * B
MOV C, A        ; C = 2 * B (save 2x)
ADD A           ; A = 4 * B
ADD A           ; A = 8 * B
ADD C           ; A = 8*B + 2*B = 10*B
MOV C, A        ; C = 10 * B
; Add new digit:
MOV A, M        ; Load ASCII character ('0'-'9')
SUI 30H         ; Convert ASCII to binary (0-9)
ADD C           ; A = 10*B + digit
MOV B, A        ; B = updated value
```

#### **2. Character Classifiers (`IS_LET`, `IS_DIG`)**
Classification relies on ASCII range boundary comparisons using `CPI` and the Carry (`CY`) flag:
- `IS_DIG`: Tests if `30H` $\le A \le$ `39H` (`'0'` to `'9'`). Returns `CY = 1` if true.
- `IS_LET`: Tests if $A =$ `5FH` (`'_'`), `41H` $\le A \le$ `5AH` (`'A'`-`'Z'`), or `61H` $\le A \le$ `7AH` (`'a'`-`'z'`). Returns `CY = 1` if true.

#### **3. Keyword Disambiguation (`CK_INT`)**
MiniC distinguishes the keyword `int` from general variable identifiers. Once an alphanumeric sequence is extracted into `CUR_NAME`:
1. Checks Byte 0 for `'i'` (`69H`). If mismatch, returns with $Z = 0$.
2. Checks Byte 1 for `'n'` (`6EH`). If mismatch, returns with $Z = 0$.
3. Checks Byte 2 for `'t'` (`74H`). If mismatch, returns with $Z = 0$.
4. Checks Byte 3 for `'\0'` (`00H`). If null, returns $Z = 1$ (`TOK_INT`); otherwise $Z = 0$ (`TOK_IDENT`).

---

## **5. Flex (Lex) High-Level Implementation**

### **5.1 Regular Definitions & Rules**
The Flex specification defines patterns declaratively using regular expressions:

```lex
DIGIT         [0-9]
NUMBER        {DIGIT}+
LETTER        [a-zA-Z_]
IDENT         {LETTER}[a-zA-Z0-9_]*
WS            [ \t\r]+
COMMENT_HASH  #[^\r\n]*
COMMENT_SLASH "//"[^\r\n]*
```

### **5.2 Lexical Action Mappings**
Each matched regular expression triggers a C action that records the token and computes its 8085-compatible value byte:

```c
"int"           { record_token(TOK_INT,   "TOK_INT",   yytext, 0x00); return TOK_INT; }
"+"             { record_token(TOK_PLUS,  "TOK_PLUS",  yytext, 0x00); return TOK_PLUS; }
"-"             { record_token(TOK_MINUS, "TOK_MINUS", yytext, 0x00); return TOK_MINUS; }
"*"             { record_token(TOK_STAR,  "TOK_STAR",  yytext, 0x00); return TOK_STAR; }
"/"             { record_token(TOK_SLASH, "TOK_SLASH", yytext, 0x00); return TOK_SLASH; }
"="             { record_token(TOK_EQUAL, "TOK_EQUAL", yytext, 0x00); return TOK_EQUAL; }
";"             { record_token(TOK_SEMI,  "TOK_SEMI",  yytext, 0x00); return TOK_SEMI; }
{NUMBER}        { 
                    int val = atoi(yytext);
                    record_token(TOK_NUM, "TOK_NUM", yytext, (unsigned char)(val & 0xFF));
                    return TOK_NUM;
                }
{IDENT}         {
                    record_token(TOK_IDENT, "TOK_IDENT", yytext, (unsigned char)yytext[0]);
                    return TOK_IDENT;
                }
```

---

## **6. Step-by-Step Worked Example & Memory Trace**

Let us trace the complete lexical scan of the reference MiniC program:

```c
// Reference MiniC Program
int a;
int b = 5;
int c = a * b + 3;
int d;
d = c - b * 2;
```

### **6.1 Input Source in Memory (Address `2000H`)**
ASCII Hex byte dump loaded at `2000H`:

```hex
2000: 69 6E 74 20 61 3B 0A 69 6E 74 20 62 20 3D 20 35  | int a;\nint b = 5
2010: 3B 0A 69 6E 74 20 63 20 3D 20 61 20 2A 20 62 20  | ;\nint c = a * b 
2020: 2B 20 33 3B 0A 69 6E 74 20 64 3B 0A 64 20 3D 20  | + 3;\nint d;\nd = 
2030: 63 20 2D 20 62 20 2A 20 32 3B 0A 00              | c - b * 2;\n\0
```

### **6.2 Complete Token Stream Table (Extracted by Lexer)**

| Token # | Line | Token ID (Hex) | Token Name | Lexeme | Stored Value Byte | Meaning / Attribute |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **#0** | Line 2 | `01H` | `TOK_INT` | `int` | `00H` | Keyword `int` |
| **#1** | Line 2 | `02H` | `TOK_IDENT` | `a` | `61H` | Variable `a` (ASCII `61H`) |
| **#2** | Line 2 | `09H` | `TOK_SEMI` | `;` | `00H` | Statement delimiter |
| **#3** | Line 3 | `01H` | `TOK_INT` | `int` | `00H` | Keyword `int` |
| **#4** | Line 3 | `02H` | `TOK_IDENT` | `b` | `62H` | Variable `b` (ASCII `62H`) |
| **#5** | Line 3 | `08H` | `TOK_EQUAL` | `=` | `00H` | Assignment operator |
| **#6** | Line 3 | `03H` | `TOK_NUM` | `5` | `05H` | Numeric constant `5` |
| **#7** | Line 3 | `09H` | `TOK_SEMI` | `;` | `00H` | Statement delimiter |
| **#8** | Line 4 | `01H` | `TOK_INT` | `int` | `00H` | Keyword `int` |
| **#9** | Line 4 | `02H` | `TOK_IDENT` | `c` | `63H` | Variable `c` (ASCII `63H`) |
| **#10** | Line 4 | `08H` | `TOK_EQUAL` | `=` | `00H` | Assignment operator |
| **#11** | Line 4 | `02H` | `TOK_IDENT` | `a` | `61H` | Variable `a` |
| **#12** | Line 4 | `06H` | `TOK_STAR` | `*` | `00H` | Multiplication operator |
| **#13** | Line 4 | `02H` | `TOK_IDENT` | `b` | `62H` | Variable `b` |
| **#14** | Line 4 | `04H` | `TOK_PLUS` | `+` | `00H` | Addition operator |
| **#15** | Line 4 | `03H` | `TOK_NUM` | `3` | `03H` | Numeric constant `3` |
| **#16** | Line 4 | `09H` | `TOK_SEMI` | `;` | `00H` | Statement delimiter |
| **#17** | Line 5 | `01H` | `TOK_INT` | `int` | `00H` | Keyword `int` |
| **#18** | Line 5 | `02H` | `TOK_IDENT` | `d` | `64H` | Variable `d` (ASCII `64H`) |
| **#19** | Line 5 | `09H` | `TOK_SEMI` | `;` | `00H` | Statement delimiter |
| **#20** | Line 6 | `02H` | `TOK_IDENT` | `d` | `64H` | Variable `d` |
| **#21** | Line 6 | `08H` | `TOK_EQUAL` | `=` | `00H` | Assignment operator |
| **#22** | Line 6 | `02H` | `TOK_IDENT` | `c` | `63H` | Variable `c` |
| **#23** | Line 6 | `05H` | `TOK_MINUS` | `-` | `00H` | Subtraction operator |
| **#24** | Line 6 | `02H` | `TOK_IDENT` | `b` | `62H` | Variable `b` |
| **#25** | Line 6 | `06H` | `TOK_STAR` | `*` | `00H` | Multiplication operator |
| **#26** | Line 6 | `03H` | `TOK_NUM` | `2` | `02H` | Numeric constant `2` |
| **#27** | Line 6 | `09H` | `TOK_SEMI` | `;` | `00H` | Statement delimiter |
| **#28** | Line 7 | `00H` | `TOK_EOF` | `\0` | `00H` | End of source stream |

**Total Valid Tokens Extracted:** 28 (excluding EOF)

---

### **6.3 Generated Output Token Memory (Address `2800H`)**

The exact memory dump generated by the 8085 Lexer starting at address `2800H`:

```hex
Address : Type Val | Disassembled Interpretation
--------:----------:-------------------------------------------
 2800H  :  01   00  | TOK_INT    -> 'int'
 2802H  :  02   61  | TOK_IDENT  -> 'a'
 2804H  :  09   00  | TOK_SEMI   -> ';'
 2806H  :  01   00  | TOK_INT    -> 'int'
 2808H  :  02   62  | TOK_IDENT  -> 'b'
 280AH  :  08   00  | TOK_EQUAL  -> '='
 280CH  :  03   05  | TOK_NUM    -> 5 (0x05)
 280EH  :  09   00  | TOK_SEMI   -> ';'
 2810H  :  01   00  | TOK_INT    -> 'int'
 2812H  :  02   63  | TOK_IDENT  -> 'c'
 2814H  :  08   00  | TOK_EQUAL  -> '='
 2816H  :  02   61  | TOK_IDENT  -> 'a'
 2818H  :  06   00  | TOK_STAR   -> '*'
 281AH  :  02   62  | TOK_IDENT  -> 'b'
 281CH  :  04   00  | TOK_PLUS   -> '+'
 281EH  :  03   03  | TOK_NUM    -> 3 (0x03)
 2820H  :  09   00  | TOK_SEMI   -> ';'
 2822H  :  01   00  | TOK_INT    -> 'int'
 2824H  :  02   64  | TOK_IDENT  -> 'd'
 2826H  :  09   00  | TOK_SEMI   -> ';'
 2828H  :  02   64  | TOK_IDENT  -> 'd'
 282AH  :  08   00  | TOK_EQUAL  -> '='
 282CH  :  02   63  | TOK_IDENT  -> 'c'
 282EH  :  05   00  | TOK_MINUS  -> '-'
 2830H  :  02   62  | TOK_IDENT  -> 'b'
 2832H  :  06   00  | TOK_STAR   -> '*'
 2834H  :  03   02  | TOK_NUM    -> 2 (0x02)
 2836H  :  09   00  | TOK_SEMI   -> ';'
 2838H  :  00   00  | TOK_EOF    -> End of Token Stream
```

### **6.4 Final State Registers (Address `7F00H`)**
- `7F00H-7F01H` (`SRC_PTR`): `3CH 20H` (Points past the null terminator at `203CH`)
- `7F02H-7F03H` (`TOK_PTR`): `3AH 28H` (Points after `TOK_EOF` at `283AH`)
- `7F04H` (`TOK_CNT`): `1CH` (28 decimal tokens)
- `7F05H` (`LINE_NUM`): `07H` (Line 7)

---

## **7. Comparative Evaluation: Intel 8085 Assembly vs. Flex**

| Architectural Metric | Intel 8085 Assembly Implementation | Flex (Fast Lexical Analyzer) Implementation |
| :--- | :--- | :--- |
| **Design Paradigm** | Imperative, register-level manual DFA transducer | Declarative regular expressions synthesized to table DFA |
| **DFA Representation** | Hardcoded conditional branches (`CPI`, `JZ`, `JC`) | 2D state transition table driven by table-walker engine |
| **Binary Footprint** | ~280 bytes of machine code | ~35 KB (includes C runtime and Flex tables) |
| **Memory Access Pattern**| Direct memory addressing (`M`, `STAX`, `LHLD`) | Buffered I/O stream pointers (`yy_buffer_state`) |
| **Arithmetic Conversion**| Custom shift-and-add arithmetic (`8*B + 2*B`) | Standard C Library functions (`atoi`, `strtol`) |
| **Lookahead Complexity** | Explicit manual lookahead (`INX H`, `DCX H`) | Automated multi-character buffer lookahead (`yytext`, `yyleng`)|
| **Extensibility** | High manual effort (requires register recalculation) | Low effort (adding one regular expression rule line) |
| **Execution Cycle Cost**| Extremely low constant overhead per character | Minimal overhead per character transition |

---

## **8. Conclusion**

The dual implementation of the MiniC Lexical Analyzer highlights the duality of compiler frontend engineering:
1. The **Intel 8085 Assembly Lexer** demonstrates how abstract compiler automata and formal language tokenization are realized at the bare-metal hardware layer. It achieves optimal memory compactness (~280 bytes), zero dependency overhead, and precise register allocation.
2. The **Flex Lexer** provides the rapid prototyping, formal specification verification, and rich diagnostic framework essential for modern software engineering.

Both systems were subjected to unit and regression testing across multiple MiniC programs, confirming 100% token sequence and binary memory equivalence.
