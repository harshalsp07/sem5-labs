;=================================================================
; MiniC-to-8085 Compiler
;
; A compiler written ENTIRELY in 8085 assembly language.
; It reads a miniature C program (ASCII) from memory and
; generates 8085 machine code (hex bytes) to another region.
;
; Supported Mini-C Grammar:
;   program      -> decl_or_stmt*
;   decl_or_stmt -> decl ';' | stmt ';'
;   decl         -> 'int' IDENT ('=' expr)?
;   stmt         -> IDENT '=' expr
;   expr         -> term (('+' | '-') term)*
;   term         -> factor (('*' | '/') factor)*
;   factor       -> IDENT | NUMBER
;
; Memory Map:
;   2000H-27FFH  Source code (ASCII text, null-terminated)
;   2800H-2FFFH  Generated machine code output
;   3000H-37FFH  Symbol table (compiler internal)
;   3800H-3FFFH  Variable storage (used at runtime)
;   4000H+       Compiler code (this program)
;   FFFFH (down) Stack
;
; Usage:
;   1. Load this compiler starting at 4000H
;   2. Load mini-C source (ASCII) at 2000H, null-terminated
;   3. Set PC = 4000H and RUN -> compiler generates code at 2800H
;   4. Set PC = 2800H and RUN -> executes the compiled program
;   5. Examine variable values starting at 3800H
;
; Generated Output Layout at 2800H:
;   2800H: JMP 2822H          (skip over helper routines)
;   2803H: MUL8 subroutine    (8-bit unsigned multiply)
;   2815H: DIV8 subroutine    (8-bit unsigned divide)
;   2822H: <compiled code>    (starts here)
;   .....: HLT                (end of compiled program)
;=================================================================

; ================= MEMORY CONSTANTS =================

SRC_BASE    EQU 2000H    ; Source code base address
OUT_BASE    EQU 2800H    ; Output code base address
SYM_BASE    EQU 3000H    ; Symbol table base address
VAR_BASE    EQU 3800H    ; Variable storage base address

; ================= TOKEN TYPE CODES =================

TOK_EOF     EQU 00H      ; End of source
TOK_INT     EQU 01H      ; Keyword 'int'
TOK_IDENT   EQU 02H      ; Identifier (variable name)
TOK_NUM     EQU 03H      ; Numeric literal
TOK_PLUS    EQU 04H      ; '+'
TOK_MINUS   EQU 05H      ; '-'
TOK_STAR    EQU 06H      ; '*'
TOK_SLASH   EQU 07H      ; '/'
TOK_EQUAL   EQU 08H      ; '='
TOK_SEMI    EQU 09H      ; ';'

; ================= COMPILER STATE VARIABLES =================
; Stored in high memory (7F00H-7F1FH)

SRC_PTR     EQU 7F00H    ; 2 bytes: current read position in source
OUT_PTR     EQU 7F02H    ; 2 bytes: current write position in output
SYM_CNT     EQU 7F04H    ; 1 byte:  number of symbols in table
VAR_NEXT    EQU 7F05H    ; 2 bytes: next free variable address
TOK_TYPE    EQU 7F07H    ; 1 byte:  type of current token
TOK_VAL     EQU 7F08H    ; 1 byte:  value of current numeric token
TOK_NAME    EQU 7F09H    ; 9 bytes: name of current identifier (max 8 + null)
SYM_NPTR    EQU 7F12H    ; 2 bytes: next free slot in symbol table

; ================= FIXED OUTPUT ADDRESSES =================

MUL_ADDR    EQU 2803H    ; Address of MUL8 in generated output
DIV_ADDR    EQU 2815H    ; Address of DIV8 in generated output
CODE_STRT   EQU 2822H    ; Address where compiled code begins

; =============================================================
;                      COMPILER CODE
; =============================================================

            ORG 4000H

;----------------------------------------------------------
; MAIN - Compiler Entry Point
;----------------------------------------------------------
MAIN:
            LXI SP, 0FFFFH      ; Initialize stack pointer

            ; Set source pointer to start of source code
            LXI H, SRC_BASE
            SHLD SRC_PTR

            ; Set output pointer to start of output area
            LXI H, OUT_BASE
            SHLD OUT_PTR

            ; Set symbol table pointer to start of symbol table
            LXI H, SYM_BASE
            SHLD SYM_NPTR

            ; Set next variable address to start of variable area
            LXI H, VAR_BASE
            SHLD VAR_NEXT

            ; Clear symbol count and token type
            XRA A
            STA SYM_CNT
            STA TOK_TYPE

            ; Emit preamble: JMP + MUL8 + DIV8 routines
            CALL EMIT_PRE

            ; Parse the entire source program
            CALL PARSE_PG

            ; Compilation complete
            HLT

;----------------------------------------------------------
; EMIT_PRE - Emit Preamble to Output
; Writes the JMP instruction and MUL8/DIV8 helper routines
; as the first 34 (22H) bytes of the generated output.
;----------------------------------------------------------
EMIT_PRE:
            LXI H, PRE_DAT      ; Point to preamble byte table
            MVI B, 22H           ; 34 bytes to emit
EP_LP:
            MOV A, M             ; Read next byte from table
            CALL EMIT_BT         ; Write it to output
            INX H                ; Advance table pointer
            DCR B                ; Decrement counter
            JNZ EP_LP            ; Loop until all 34 bytes written
            RET

; ---- Preamble Data Table (34 bytes) ----
;
; Byte layout of the generated preamble:
;
; 2800: C3 22 28       JMP 2822H    ; Skip to compiled code
;
; --- MUL8 routine (A = A * B, unsigned 8-bit) ---
; 2803: 4F             MOV C, A     ; C = multiplicand
; 2804: 78             MOV A, B     ; A = multiplier
; 2805: B7             ORA A        ; Test if multiplier == 0
; 2806: CA 12 28       JZ  2812H    ; If zero, return 0
; 2809: 57             MOV D, A     ; D = loop counter
; 280A: 3E 00          MVI A, 00H   ; Result = 0
; 280C: 81             ADD C        ; Result += multiplicand (loop)
; 280D: 15             DCR D        ; Counter--
; 280E: C2 0C 28       JNZ 280CH    ; Repeat until counter = 0
; 2811: C9             RET          ; Return result in A
; 2812: 3E 00          MVI A, 00H   ; Multiplier was 0, result = 0
; 2814: C9             RET
;
; --- DIV8 routine (A = A / B, unsigned 8-bit) ---
; 2815: 0E 00          MVI C, 00H   ; C = quotient = 0
; 2817: B8             CMP B        ; Compare dividend with divisor
; 2818: DA 20 28       JC  2820H    ; If dividend < divisor, done
; 281B: 90             SUB B        ; Dividend -= divisor
; 281C: 0C             INR C        ; Quotient++
; 281D: C3 17 28       JMP 2817H    ; Repeat
; 2820: 79             MOV A, C     ; A = quotient
; 2821: C9             RET
;
PRE_DAT:
            ; JMP 2822H
            DB 0C3H, 22H, 28H
            ; MUL8: MOV C,A; MOV A,B; ORA A; JZ 2812H
            DB 4FH, 78H, 0B7H, 0CAH, 12H, 28H
            ; MOV D,A; MVI A,00H
            DB 57H, 3EH, 00H
            ; ADD C; DCR D; JNZ 280CH
            DB 81H, 15H, 0C2H, 0CH, 28H
            ; RET
            DB 0C9H
            ; MVI A,00H; RET  (MUL_ZERO)
            DB 3EH, 00H, 0C9H
            ; DIV8: MVI C,00H
            DB 0EH, 00H
            ; CMP B; JC 2820H
            DB 0B8H, 0DAH, 20H, 28H
            ; SUB B; INR C; JMP 2817H
            DB 90H, 0CH, 0C3H, 17H, 28H
            ; MOV A,C; RET  (DIV_DONE)
            DB 79H, 0C9H

; =============================================================
;                         PARSER
; =============================================================

;----------------------------------------------------------
; PARSE_PG - Parse Program
; program -> decl_or_stmt*
; Loops until EOF, dispatching to PARSE_DC or PARSE_ST
;----------------------------------------------------------
PARSE_PG:
            CALL GET_TK          ; Read first token
PG_LP:
            LDA TOK_TYPE
            CPI TOK_EOF          ; End of source?
            JZ PG_DN
            CPI TOK_INT          ; Declaration?
            JZ PG_DC
            ; Otherwise assume assignment statement
            CALL PARSE_ST
            JMP PG_LP
PG_DC:
            CALL PARSE_DC
            JMP PG_LP
PG_DN:
            ; Emit HLT (76H) at end of compiled program
            MVI A, 76H
            CALL EMIT_BT
            RET

;----------------------------------------------------------
; PARSE_DC - Parse Declaration
; decl -> 'int' IDENT ('=' expr)?
;
; Adds variable to symbol table. If initializer present,
; emits code for expression followed by STA to variable.
;----------------------------------------------------------
PARSE_DC:
            ; Current token is TOK_INT
            CALL GET_TK          ; Read identifier name
            ; Now TOK_TYPE=IDENT, TOK_NAME has variable name

            CALL SYM_ADD         ; Add to symbol table -> DE = variable addr
            PUSH D               ; Save variable address on stack

            CALL GET_TK          ; Read next token ('=' or ';')
            LDA TOK_TYPE
            CPI TOK_EQUAL
            JNZ PD_NI            ; No initializer -> jump

            ; Has initializer: parse expression
            CALL GET_TK          ; Read first token of expression
            CALL PARSE_EX        ; Emit code for expression (result in A)
            POP D                ; Restore variable address

            ; Emit: STA <variable_address>
            MVI A, 32H           ; STA opcode
            CALL EMIT_BT
            MOV A, E             ; Address low byte
            CALL EMIT_BT
            MOV A, D             ; Address high byte
            CALL EMIT_BT
            JMP PD_SM

PD_NI:
            POP D                ; Discard saved address (no init code)

PD_SM:
            ; Current token should be ';'
            CALL GET_TK          ; Advance past ';'
            RET

;----------------------------------------------------------
; PARSE_ST - Parse Statement
; stmt -> IDENT '=' expr
;
; Looks up variable, emits code for expression, then STA.
;----------------------------------------------------------
PARSE_ST:
            ; Current token is TOK_IDENT
            CALL SYM_LK         ; Look up variable -> DE = address
            PUSH D               ; Save variable address

            CALL GET_TK          ; Read '=' token
            ; TOK_TYPE should be TOK_EQUAL

            CALL GET_TK          ; Read first token of expression
            CALL PARSE_EX        ; Emit code for expression
            POP D                ; Restore variable address

            ; Emit: STA <variable_address>
            MVI A, 32H           ; STA opcode
            CALL EMIT_BT
            MOV A, E             ; Address low byte
            CALL EMIT_BT
            MOV A, D             ; Address high byte
            CALL EMIT_BT

            ; Current token should be ';'
            CALL GET_TK          ; Advance past ';'
            RET

;----------------------------------------------------------
; PARSE_EX - Parse Expression
; expr -> term (('+' | '-') term)*
;
; Generated code pattern for A + B:
;   <code for A>     ; result in accumulator
;   PUSH PSW         ; save A
;   <code for B>     ; result in accumulator
;   MOV B, A         ; B = second operand
;   POP PSW          ; A = first operand
;   ADD B            ; A = A + B
;----------------------------------------------------------
PARSE_EX:
            CALL PARSE_TM        ; Parse first term
EX_LP:
            LDA TOK_TYPE
            CPI TOK_PLUS
            JZ EX_AD
            CPI TOK_MINUS
            JZ EX_SB
            RET                  ; Not + or - -> expression done

EX_AD:
            ; Emit PUSH PSW to save first operand
            MVI A, 0F5H
            CALL EMIT_BT

            CALL GET_TK          ; Advance past '+'
            CALL PARSE_TM        ; Parse next term

            ; Emit: MOV B,A; POP PSW; ADD B
            MVI A, 47H           ; MOV B, A
            CALL EMIT_BT
            MVI A, 0F1H          ; POP PSW
            CALL EMIT_BT
            MVI A, 80H           ; ADD B
            CALL EMIT_BT
            JMP EX_LP

EX_SB:
            ; Emit PUSH PSW to save first operand
            MVI A, 0F5H
            CALL EMIT_BT

            CALL GET_TK          ; Advance past '-'
            CALL PARSE_TM        ; Parse next term

            ; Emit: MOV B,A; POP PSW; SUB B
            MVI A, 47H           ; MOV B, A
            CALL EMIT_BT
            MVI A, 0F1H          ; POP PSW
            CALL EMIT_BT
            MVI A, 90H           ; SUB B
            CALL EMIT_BT
            JMP EX_LP

;----------------------------------------------------------
; PARSE_TM - Parse Term
; term -> factor (('*' | '/') factor)*
;
; Same pattern as PARSE_EX but uses CALL MUL8/DIV8.
;----------------------------------------------------------
PARSE_TM:
            CALL PARSE_FC        ; Parse first factor
TM_LP:
            LDA TOK_TYPE
            CPI TOK_STAR
            JZ TM_ML
            CPI TOK_SLASH
            JZ TM_DV
            RET                  ; Not * or / -> term done

TM_ML:
            ; Emit PUSH PSW
            MVI A, 0F5H
            CALL EMIT_BT

            CALL GET_TK          ; Advance past '*'
            CALL PARSE_FC        ; Parse next factor

            ; Emit: MOV B,A; POP PSW; CALL MUL8
            MVI A, 47H           ; MOV B, A
            CALL EMIT_BT
            MVI A, 0F1H          ; POP PSW
            CALL EMIT_BT
            MVI A, 0CDH          ; CALL opcode
            CALL EMIT_BT
            MVI A, 03H           ; MUL_ADDR low byte (2803H)
            CALL EMIT_BT
            MVI A, 28H           ; MUL_ADDR high byte
            CALL EMIT_BT
            JMP TM_LP

TM_DV:
            ; Emit PUSH PSW
            MVI A, 0F5H
            CALL EMIT_BT

            CALL GET_TK          ; Advance past '/'
            CALL PARSE_FC        ; Parse next factor

            ; Emit: MOV B,A; POP PSW; CALL DIV8
            MVI A, 47H           ; MOV B, A
            CALL EMIT_BT
            MVI A, 0F1H          ; POP PSW
            CALL EMIT_BT
            MVI A, 0CDH          ; CALL opcode
            CALL EMIT_BT
            MVI A, 15H           ; DIV_ADDR low byte (2815H)
            CALL EMIT_BT
            MVI A, 28H           ; DIV_ADDR high byte
            CALL EMIT_BT
            JMP TM_LP

;----------------------------------------------------------
; PARSE_FC - Parse Factor
; factor -> IDENT | NUMBER
;----------------------------------------------------------
PARSE_FC:
            LDA TOK_TYPE
            CPI TOK_IDENT
            JZ FC_ID
            CPI TOK_NUM
            JZ FC_NM
            ; Unexpected token - just return (error case)
            RET

FC_ID:
            ; Variable reference
            CALL SYM_LK         ; Look up variable -> DE = address
            ; Emit: LDA <variable_address>
            MVI A, 3AH           ; LDA opcode
            CALL EMIT_BT
            MOV A, E             ; Address low byte
            CALL EMIT_BT
            MOV A, D             ; Address high byte
            CALL EMIT_BT
            CALL GET_TK          ; Advance past identifier
            RET

FC_NM:
            ; Numeric literal
            ; Emit: MVI A, <value>
            MVI A, 3EH           ; MVI A opcode
            CALL EMIT_BT
            LDA TOK_VAL          ; Load numeric value
            CALL EMIT_BT
            CALL GET_TK          ; Advance past number
            RET

; =============================================================
;                          LEXER
; =============================================================

;----------------------------------------------------------
; GET_TK - Get Next Token (Lexer)
; Scans source from SRC_PTR, classifies the next token.
; Sets: TOK_TYPE, and TOK_VAL or TOK_NAME as applicable.
; Preserves: HL, DE, BC (caller's registers)
;----------------------------------------------------------
GET_TK:
            PUSH H
            PUSH D
            PUSH B

            LHLD SRC_PTR         ; HL = current source position

            ; ---- Skip whitespace ----
GT_WS:
            MOV A, M
            CPI 20H              ; Space
            JZ GT_WN
            CPI 09H              ; Tab
            JZ GT_WN
            CPI 0AH              ; Newline (LF)
            JZ GT_WN
            CPI 0DH              ; Carriage return
            JZ GT_WN
            JMP GT_CK             ; Not whitespace -> classify
GT_WN:
            INX H                 ; Skip whitespace character
            JMP GT_WS

            ; ---- Classify character ----
GT_CK:
            ; Check for end of source (null byte)
            ORA A
            JNZ GT_NE
            MVI A, TOK_EOF
            STA TOK_TYPE
            JMP GT_DN

GT_NE:
            ; Check for letter -> identifier or keyword
            CALL IS_LET
            JC GT_RI              ; Read identifier

            ; Check for digit -> number
            CALL IS_DIG
            JC GT_RN              ; Read number

            ; Check single-character tokens
            CPI 2BH              ; '+'
            JZ GT_PL
            CPI 2DH              ; '-'
            JZ GT_MI
            CPI 2AH              ; '*'
            JZ GT_SR
            CPI 2FH              ; '/'
            JZ GT_SL
            CPI 3DH              ; '='
            JZ GT_EQ
            CPI 3BH              ; ';'
            JZ GT_SE

            ; Unknown character -> skip it
            INX H
            JMP GT_WS

            ; ---- Single-character token handlers ----
GT_PL:      MVI A, TOK_PLUS
            STA TOK_TYPE
            INX H
            JMP GT_DN
GT_MI:      MVI A, TOK_MINUS
            STA TOK_TYPE
            INX H
            JMP GT_DN
GT_SR:      MVI A, TOK_STAR
            STA TOK_TYPE
            INX H
            JMP GT_DN
GT_SL:      MVI A, TOK_SLASH
            STA TOK_TYPE
            INX H
            JMP GT_DN
GT_EQ:      MVI A, TOK_EQUAL
            STA TOK_TYPE
            INX H
            JMP GT_DN
GT_SE:      MVI A, TOK_SEMI
            STA TOK_TYPE
            INX H
            JMP GT_DN

            ; ---- Read Identifier ----
GT_RI:
            LXI D, TOK_NAME      ; DE = name buffer
            MVI C, 08H           ; Max 8 characters
RI_LP:
            MOV A, M              ; A = current char
            CALL IS_LET
            JC RI_ST              ; Letter -> store it
            CALL IS_DIG
            JC RI_ST              ; Digit -> store it
            JMP RI_DN              ; Not alphanumeric -> end
RI_ST:
            STAX D                ; Store char in TOK_NAME
            INX H                 ; Next source character
            INX D                 ; Next buffer position
            DCR C                 ; Decrement limit
            JNZ RI_LP             ; Continue if limit not hit
            ; Max reached - skip any remaining alphanumeric chars
RI_SK:
            MOV A, M
            CALL IS_LET
            JC RI_S2
            CALL IS_DIG
            JC RI_S2
            JMP RI_DN
RI_S2:
            INX H                 ; Skip extra char
            JMP RI_SK

RI_DN:
            ; Null-terminate the identifier name
            XRA A                 ; A = 0
            STAX D                ; Store null terminator

            ; Check if the identifier is the keyword "int"
            CALL CK_INT
            JZ RI_KW              ; Yes -> it's a keyword

            MVI A, TOK_IDENT      ; No -> it's an identifier
            STA TOK_TYPE
            JMP GT_DN

RI_KW:
            MVI A, TOK_INT        ; Token type = keyword 'int'
            STA TOK_TYPE
            JMP GT_DN

            ; ---- Read Number ----
GT_RN:
            MVI B, 00H            ; B = accumulated numeric value
RN_LP:
            MOV A, M              ; A = current character
            CALL IS_DIG
            JNC RN_DN              ; Not a digit -> done

            ; Multiply accumulated value by 10:
            ;   value*10 = value*2*2*2 + value*2
            ;   Shortcut: val, 2*val, 4*val, 5*val, 10*val
            MOV A, B              ; A = current value
            MOV C, A              ; C = save value
            ADD A                 ; A = 2 * value
            ADD A                 ; A = 4 * value
            ADD C                 ; A = 5 * value
            ADD A                 ; A = 10 * value
            MOV C, A              ; C = 10 * value

            ; Add new digit
            MOV A, M              ; A = ASCII digit character
            SUI 30H               ; Convert ASCII to digit (0-9)
            ADD C                 ; A = 10*value + digit
            MOV B, A              ; B = updated value

            INX H                 ; Next source character
            JMP RN_LP

RN_DN:
            MOV A, B
            STA TOK_VAL           ; Store numeric value
            MVI A, TOK_NUM
            STA TOK_TYPE
            JMP GT_DN

            ; ---- Done ----
GT_DN:
            SHLD SRC_PTR          ; Save updated source pointer
            POP B
            POP D
            POP H
            RET

; =============================================================
;                      SYMBOL TABLE
; =============================================================

;----------------------------------------------------------
; SYM_LK - Symbol Table Lookup
; Searches for TOK_NAME in the symbol table.
; Output: DE = variable memory address (in 3800H+ region)
;         Carry flag: set if found, clear if not found
;----------------------------------------------------------
SYM_LK:
            PUSH H
            PUSH B

            LDA SYM_CNT
            ORA A
            JZ SL_NF              ; No symbols -> not found
            MOV B, A              ; B = number of entries to check
            LXI H, SYM_BASE      ; HL = first entry

SL_LP:
            PUSH H                ; Save entry start
            PUSH B                ; Save counter
            CALL STR_CM           ; Compare [HL] with TOK_NAME
            POP B                 ; Restore counter
            POP H                 ; Restore entry start
            JZ SL_FD              ; Z set -> names match!

            ; Advance to next entry (each entry = 11 bytes)
            LXI D, 000BH
            DAD D                 ; HL += 11
            DCR B
            JNZ SL_LP             ; Check next entry

SL_NF:
            ; Not found
            POP B
            POP H
            STC
            CMC                   ; Clear carry = not found
            RET

SL_FD:
            ; Found! Variable address is at offset 9 in entry
            LXI D, 0009H
            DAD D                 ; HL -> address field
            MOV E, M              ; E = address low byte
            INX H
            MOV D, M              ; D = address high byte
            STC                   ; Set carry = found
            POP B
            POP H
            RET

;----------------------------------------------------------
; SYM_ADD - Add Symbol to Table
; Copies TOK_NAME into the next free symbol table slot,
; allocates a variable storage byte, returns its address.
; Output: DE = allocated variable memory address
;----------------------------------------------------------
SYM_ADD:
            PUSH H
            PUSH B

            LHLD SYM_NPTR        ; HL = next free slot in table

            ; Copy 9 bytes from TOK_NAME to entry name field
            LXI D, TOK_NAME
            MVI C, 09H
SA_CP:
            LDAX D                ; A = byte from TOK_NAME
            MOV M, A              ; Write to symbol table
            INX H
            INX D
            DCR C
            JNZ SA_CP

            ; HL now points to the address field (offset 9)
            PUSH H                ; Save address-field pointer

            ; Allocate next variable address
            LHLD VAR_NEXT         ; HL = next free variable address
            XCHG                  ; DE = variable address (return value)
            LXI H, 0001H
            DAD D                 ; HL = next address + 1
            SHLD VAR_NEXT         ; Update for next allocation

            POP H                 ; HL = address-field pointer
            MOV M, E              ; Store variable addr low byte
            INX H
            MOV M, D              ; Store variable addr high byte
            INX H                 ; HL = past this entry
            SHLD SYM_NPTR         ; Update next free slot pointer

            ; Increment symbol count
            LDA SYM_CNT
            INR A
            STA SYM_CNT

            ; DE = allocated variable address (already set)
            POP B
            POP H
            RET

; =============================================================
;                     UTILITY ROUTINES
; =============================================================

;----------------------------------------------------------
; IS_LET - Is A a Letter?
; Returns carry set if A is 'A'-'Z' or 'a'-'z'.
; A is preserved.
;----------------------------------------------------------
IS_LET:
            CPI 41H              ; >= 'A' ?
            JC IL_NO
            CPI 5BH              ; <= 'Z' ?  (5BH = 'Z'+1)
            JC IL_YES
            CPI 61H              ; >= 'a' ?
            JC IL_NO
            CPI 7BH              ; <= 'z' ?  (7BH = 'z'+1)
            JC IL_YES
IL_NO:
            STC
            CMC                   ; Clear carry (not a letter)
            RET
IL_YES:
            STC                   ; Set carry (is a letter)
            RET

;----------------------------------------------------------
; IS_DIG - Is A a Digit?
; Returns carry set if A is '0'-'9'.
; A is preserved.
;----------------------------------------------------------
IS_DIG:
            CPI 30H              ; >= '0' ?
            JC ID_NO
            CPI 3AH              ; <= '9' ?  (3AH = '9'+1)
            JC ID_YES
ID_NO:
            STC
            CMC                   ; Clear carry (not a digit)
            RET
ID_YES:
            STC                   ; Set carry (is a digit)
            RET

;----------------------------------------------------------
; CK_INT - Check if TOK_NAME equals "int"
; Returns Z flag set if TOK_NAME is "int\0"
;----------------------------------------------------------
CK_INT:
            PUSH H
            LXI H, TOK_NAME
            MOV A, M
            CPI 69H              ; 'i'
            JNZ CK_NO
            INX H
            MOV A, M
            CPI 6EH              ; 'n'
            JNZ CK_NO
            INX H
            MOV A, M
            CPI 74H              ; 't'
            JNZ CK_NO
            INX H
            MOV A, M
            ORA A                ; Should be 00H (null terminator)
            JNZ CK_NO
            ; Match: Z flag is set
            POP H
            XRA A                ; A = 0 -> Z = 1
            RET
CK_NO:
            ; No match: clear Z flag
            POP H
            MVI A, 01H           ; A != 0
            ORA A                ; Z = 0
            RET

;----------------------------------------------------------
; STR_CM - String Compare
; Compares null-terminated string at [HL] with TOK_NAME.
; Returns Z flag set if strings are equal.
; Destroys A, DE. HL is saved/restored by caller.
;----------------------------------------------------------
STR_CM:
            LXI D, TOK_NAME
SC_LP:
            LDAX D                ; A = char from TOK_NAME
            CMP M                 ; Compare with char at [HL]
            JNZ SC_NE             ; Different -> not equal
            ORA A                 ; Both null?
            JZ SC_EQ              ; Yes -> match!
            INX H
            INX D
            JMP SC_LP
SC_NE:
            ; Not equal (Z is clear from CMP M)
            RET
SC_EQ:
            ; Equal (Z is set from ORA A with A=0)
            RET

;----------------------------------------------------------
; EMIT_BT - Emit Byte to Output
; Writes the byte in A to the output area at OUT_PTR,
; then advances OUT_PTR by 1.
; Preserves: A, DE, BC, HL
;----------------------------------------------------------
EMIT_BT:
            PUSH H
            LHLD OUT_PTR          ; HL = output write position
            MOV M, A              ; Write byte to output
            INX H                 ; Advance position
            SHLD OUT_PTR          ; Save updated position
            POP H
            RET

;=================================================================
; End of MiniC-to-8085 Compiler
;=================================================================
