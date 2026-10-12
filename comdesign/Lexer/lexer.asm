; ==============================================================================
;                  MINIC LEXICAL ANALYZER (TOKENIZER) FOR 8085
; ==============================================================================
; Description:
;   Scans ASCII MiniC source code stored in memory starting at address 2000H
;   and converts it into a sequential stream of 2-byte token records stored
;   starting at address 2800H.
;
; Memory Layout:
;   2000H - 27FFH : Input MiniC Source Code (ASCII text, null-terminated 00H)
;   2800H - 2FFFH : Output Token Stream (2 Bytes per Token: [Type, Value])
;   4000H - 43FFH : Lexer Executable Machine Code (Program Entry Point: 4000H)
;   7F00H - 7F10H : Internal Status & State Variables
;   FFFFH (down)  : Stack Area
;
; Token Record Format in Memory (2800H+):
;   Each token is stored as a 2-byte record:
;     Byte 0 (Address 2800H + 2*k)     : Token Type Code (00H to 09H)
;     Byte 1 (Address 2800H + 2*k + 1) : Token Value / Attribute
;
; Token Type Codes:
;   00H = TOK_EOF   (End of input sentinel)  -> Value: 00H
;   01H = TOK_INT   (Keyword 'int')          -> Value: 00H
;   02H = TOK_IDENT (Identifier variable)    -> Value: ASCII code (e.g., 'a' = 61H)
;   03H = TOK_NUM   (Numeric constant 0-255) -> Value: Binary integer (e.g., 5 = 05H)
;   04H = TOK_PLUS  ('+' operator)           -> Value: 00H
;   05H = TOK_MINUS ('-' operator)           -> Value: 00H
;   06H = TOK_STAR  ('*' operator)           -> Value: 00H
;   07H = TOK_SLASH ('/' operator)           -> Value: 00H
;   08H = TOK_EQUAL ('=' operator)           -> Value: 00H
;   09H = TOK_SEMI  (';' statement delimiter)-> Value: 00H
;
; State Variables (7F00H - 7F10H):
;   7F00H - 7F01H : SRC_PTR  (2 Bytes) - Current read pointer in source code
;   7F02H - 7F03H : TOK_PTR  (2 Bytes) - Current write pointer in token memory
;   7F04H         : TOK_CNT  (1 Byte)  - Total count of extracted tokens
;   7F05H         : LINE_NUM (1 Byte)  - Current line number counter
;   7F06H         : CUR_TYPE (1 Byte)  - Type ID of current token
;   7F07H         : CUR_VAL  (1 Byte)  - Value / Attribute of current token
;   7F08H - 7F10H : CUR_NAME (9 Bytes) - Temporary identifier string buffer
; ==============================================================================
            ORG 2000H
SRC_DATA:   DB 69H, 6EH, 74H, 20H, 61H, 20H, 3DH, 20H, 35H, 3BH, 00Hq
; Memory Base Constants
SRC_BASE    EQU 2000H    ; Source code start address
TOK_BASE    EQU 2800H    ; Token output start address

; Token Type Constants
TOK_EOF     EQU 00H      ; End of input
TOK_INT     EQU 01H      ; 'int' keyword
TOK_IDENT   EQU 02H      ; Identifier
TOK_NUM     EQU 03H      ; Number (0..255)
TOK_PLUS    EQU 04H      ; '+'
TOK_MINUS   EQU 05H      ; '-'
TOK_STAR    EQU 06H      ; '*'
TOK_SLASH   EQU 07H      ; '/'
TOK_EQUAL   EQU 08H      ; '='
TOK_SEMI    EQU 09H      ; ';'

; State Variable Addresses
SRC_PTR     EQU 7F00H    ; 2 bytes: read pointer
TOK_PTR     EQU 7F02H    ; 2 bytes: write pointer
TOK_CNT     EQU 7F04H    ; 1 byte:  token count
LINE_NUM    EQU 7F05H    ; 1 byte:  line number
CUR_TYPE    EQU 7F06H    ; 1 byte:  token type
CUR_VAL     EQU 7F07H    ; 1 byte:  token value
CUR_NAME    EQU 7F08H    ; 9 bytes: identifier name

; ==============================================================================
;                             MAIN ENTRY POINT
; ==============================================================================
            ORG 4000H

START:
            ; 1. Initialize Stack Pointer to top of memory
            LXI SP, 0FFFFH

            ; 2. Initialize Source Read Pointer to 2000H
            LXI H, SRC_BASE
            SHLD SRC_PTR

            ; 3. Initialize Token Write Pointer to 2800H
            LXI H, TOK_BASE
            SHLD TOK_PTR

            ; 4. Clear Token Counter to 0
            XRA A
            STA TOK_CNT

            ; 5. Set Initial Line Number to 1
            MVI A, 01H
            STA LINE_NUM

; ------------------------------------------------------------------------------
; Main Tokenization Loop:
; Repeatedly calls GET_NEXT_TOKEN and stores (Type, Value) pairs at 2800H+
; until TOK_EOF (00H) is reached.
; ------------------------------------------------------------------------------
TOKEN_LOOP:
            CALL GET_NEXT_TOKEN ; Scan next token from source

            ; Load current write pointer into HL
            LHLD TOK_PTR

            ; Write Byte 0: Token Type
            LDA CUR_TYPE
            MOV M, A
            INX H

            ; Write Byte 1: Token Value / Attribute
            LDA CUR_VAL
            MOV M, A
            INX H

            ; Save updated write pointer
            SHLD TOK_PTR

            ; Check if EOF was emitted
            LDA CUR_TYPE
            CPI TOK_EOF
            JZ SCAN_COMPLETE    ; If EOF, finish scanning

            ; Increment total token counter
            LDA TOK_CNT
            INR A
            STA TOK_CNT

            JMP TOKEN_LOOP      ; Scan next token

SCAN_COMPLETE:
            HLT                 ; Execution Complete! Inspect 2800H in Sim8085.

; ==============================================================================
;                        LEXICAL SCANNING ENGINE
; ==============================================================================

; ------------------------------------------------------------------------------
; GET_NEXT_TOKEN:
; Reads ASCII characters from [SRC_PTR], skips whitespace & comments,
; classifies lexemes, and sets CUR_TYPE, CUR_VAL, and CUR_NAME.
; ------------------------------------------------------------------------------
GET_NEXT_TOKEN:
            PUSH H
            PUSH D
            PUSH B

            LHLD SRC_PTR        ; HL = current read position in source

; --- Phase 1: Skip Whitespace & Comments ---
SKIP_WHITESPACE:
            MOV A, M            ; Read character at [HL]

            ; Check for EOF / Null terminator (00H) or '$' (24H)
            ORA A
            JZ MATCH_EOF
            CPI 24H             ; '$' sentinel
            JZ MATCH_EOF

            ; Check Spaces, Tabs, and Carriage Returns
            CPI 20H             ; Space (' ')
            JZ ADVANCE_CHAR
            CPI 09H             ; Tab ('\t')
            JZ ADVANCE_CHAR
            CPI 0DH             ; Carriage Return ('\r')
            JZ ADVANCE_CHAR

            ; Check Newline ('\n' = 0AH) -> Increment Line Tracker
            CPI 0AH
            JNZ CHECK_COMMENTS
            LDA LINE_NUM
            INR A
            STA LINE_NUM
            JMP ADVANCE_CHAR

CHECK_COMMENTS:
            ; Check single-line hash comment '#' (23H)
            CPI 23H
            JZ SKIP_HASH_COMMENT

            ; Check single-line slash comment '//' (2FH 2FH)
            CPI 2FH
            JNZ DISPATCH_TOKEN  ; Not '/', proceed to token dispatch

            ; Look ahead at the next character
            INX H
            MOV A, M
            DCX H               ; Restore HL pointer
            CPI 2FH             ; Is the second character also '/'?
            JNZ DISPATCH_TOKEN  ; No -> it is a single '/' division operator

SKIP_HASH_COMMENT:
            ; Skip characters until '\n' or EOF
SKIP_COMMENT_LOOP:
            MOV A, M
            ORA A
            JZ MATCH_EOF
            CPI 0AH             ; Found newline
            JZ COMMENT_END
            INX H
            JMP SKIP_COMMENT_LOOP

COMMENT_END:
            INX H               ; Skip over '\n'
            LDA LINE_NUM
            INR A
            STA LINE_NUM
            JMP SKIP_WHITESPACE

ADVANCE_CHAR:
            INX H               ; Move to next source character
            JMP SKIP_WHITESPACE

; --- Phase 2: Token Classification & Dispatch ---
DISPATCH_TOKEN:
            MOV A, M            ; Reload current character

            ; 1. Check EOF
            ORA A
            JZ MATCH_EOF
            CPI 24H             ; '$'
            JZ MATCH_EOF

            ; 2. Check Identifier / Keyword: Letter ('a'-'z', 'A'-'Z') or '_'
            CALL IS_LETTER
            JC SCAN_IDENTIFIER

            ; 3. Check Number: Digit ('0'-'9')
            CALL IS_DIGIT
            JC SCAN_NUMBER

            ; 4. Check Operators & Delimiters
            CPI 2BH             ; '+'
            JZ OP_PLUS
            CPI 2DH             ; '-'
            JZ OP_MINUS
            CPI 2AH             ; '*'
            JZ OP_STAR
            CPI 2FH             ; '/'
            JZ OP_SLASH
            CPI 3DH             ; '='
            JZ OP_EQUAL
            CPI 3BH             ; ';'
            JZ OP_SEMI

            ; Unrecognized character: skip and continue scanning
            INX H
            JMP SKIP_WHITESPACE

; --- Operator Handlers ---
OP_PLUS:
            MVI A, TOK_PLUS
            STA CUR_TYPE
            XRA A
            STA CUR_VAL
            INX H
            JMP SCAN_EXIT

OP_MINUS:
            MVI A, TOK_MINUS
            STA CUR_TYPE
            XRA A
            STA CUR_VAL
            INX H
            JMP SCAN_EXIT

OP_STAR:
            MVI A, TOK_STAR
            STA CUR_TYPE
            XRA A
            STA CUR_VAL
            INX H
            JMP SCAN_EXIT

OP_SLASH:
            MVI A, TOK_SLASH
            STA CUR_TYPE
            XRA A
            STA CUR_VAL
            INX H
            JMP SCAN_EXIT

OP_EQUAL:
            MVI A, TOK_EQUAL
            STA CUR_TYPE
            XRA A
            STA CUR_VAL
            INX H
            JMP SCAN_EXIT

OP_SEMI:
            MVI A, TOK_SEMI
            STA CUR_TYPE
            XRA A
            STA CUR_VAL
            INX H
            JMP SCAN_EXIT

MATCH_EOF:
            MVI A, TOK_EOF
            STA CUR_TYPE
            XRA A
            STA CUR_VAL
            JMP SCAN_EXIT

; --- Phase 3: Identifier & Keyword Scanning ---
SCAN_IDENTIFIER:
            LXI D, CUR_NAME     ; DE = pointer to name buffer (7F08H)
            MVI C, 08H          ; Max identifier length = 8 chars
            MOV A, M
            STA CUR_VAL         ; Store ASCII of 1st char as default attribute

IDENT_LOOP:
            MOV A, M            ; Read current char
            CALL IS_LETTER
            JC IDENT_STORE
            CALL IS_DIGIT
            JC IDENT_STORE
            JMP IDENT_DONE      ; Delimiter found -> end of identifier

IDENT_STORE:
            STAX D              ; Store char in CUR_NAME buffer
            INX H               ; Advance source pointer
            INX D               ; Advance buffer pointer
            DCR C
            JNZ IDENT_LOOP

            ; If identifier length exceeds 8 chars, skip extra characters
IDENT_SKIP_EXTRA:
            MOV A, M
            CALL IS_LETTER
            JC IDENT_SKIP_ADV
            CALL IS_DIGIT
            JC IDENT_SKIP_ADV
            JMP IDENT_DONE
IDENT_SKIP_ADV:
            INX H
            JMP IDENT_SKIP_EXTRA

IDENT_DONE:
            ; Null-terminate the string in CUR_NAME
            XRA A
            STAX D

            ; Check if the scanned identifier is the keyword "int"
            CALL CHECK_KEYWORD_INT
            JZ IS_INT_KEYWORD

            ; Regular variable identifier
            MVI A, TOK_IDENT
            STA CUR_TYPE
            JMP SCAN_EXIT

IS_INT_KEYWORD:
            MVI A, TOK_INT
            STA CUR_TYPE
            XRA A
            STA CUR_VAL         ; Keyword has value 00H
            JMP SCAN_EXIT

; --- Phase 4: Numeric Literal Scanning (Base-10 Accumulation) ---
SCAN_NUMBER:
            MVI B, 00H          ; B = accumulated numeric value (0..255)

NUMBER_LOOP:
            MOV A, M            ; Read character
            CALL IS_DIGIT
            JNC NUMBER_DONE     ; Non-digit -> end of number

            ; Perform B = (B * 10) + digit using binary expansion:
            ; 10 * B = (8 * B) + (2 * B)
            MOV A, B            ; A = B
            ADD A               ; A = 2 * B
            MOV C, A            ; C = 2 * B (save 2*B)
            ADD A               ; A = 4 * B
            ADD A               ; A = 8 * B
            ADD C               ; A = 8*B + 2*B = 10*B
            MOV C, A            ; C = 10 * B

            ; Add new digit value
            MOV A, M            ; Load ASCII digit ('0'-'9')
            SUI 30H             ; Convert ASCII to binary (0-9)
            ADD C               ; A = 10*B + digit
            MOV B, A            ; B = updated accumulated value

            INX H               ; Move to next digit
            JMP NUMBER_LOOP

NUMBER_DONE:
            MOV A, B
            STA CUR_VAL         ; Store 8-bit integer value in CUR_VAL
            MVI A, TOK_NUM
            STA CUR_TYPE
            JMP SCAN_EXIT

; --- Scan Exit Subroutine ---
SCAN_EXIT:
            SHLD SRC_PTR        ; Save updated read pointer
            POP B
            POP D
            POP H
            RET

; ==============================================================================
;                           HELPER SUBROUTINES
; ==============================================================================

; ------------------------------------------------------------------------------
; IS_LETTER:
; Checks if character in Accumulator A is a letter ('a'-'z', 'A'-'Z') or '_' (5FH).
; Output: Carry flag SET (CY=1) if true, CLEAR (CY=0) if false.
; ------------------------------------------------------------------------------
IS_LETTER:
            CPI 5FH             ; '_' (Underscore)
            JZ IS_LET_TRUE

            ; Check lowercase 'a'-'z' (61H - 7AH)
            CPI 61H
            JC CHK_UPPERCASE    ; If < 'a', check uppercase
            CPI 7BH
            JC IS_LET_TRUE      ; If >= 'a' and <= 'z' -> True

CHK_UPPERCASE:
            ; Check uppercase 'A'-'Z' (41H - 5AH)
            CPI 41H
            JC IS_LET_FALSE     ; If < 'A' -> False
            CPI 5BH
            JC IS_LET_TRUE      ; If >= 'A' and <= 'Z' -> True

IS_LET_FALSE:
            STC
            CMC                 ; Clear Carry (CY = 0)
            RET

IS_LET_TRUE:
            STC                 ; Set Carry (CY = 1)
            RET

; ------------------------------------------------------------------------------
; IS_DIGIT:
; Checks if character in Accumulator A is a decimal digit ('0'-'9' = 30H-39H).
; Output: Carry flag SET (CY=1) if true, CLEAR (CY=0) if false.
; ------------------------------------------------------------------------------
IS_DIGIT:
            CPI 30H             ; < '0'
            JC IS_DIG_FALSE
            CPI 3AH             ; <= '9'
            JC IS_DIG_TRUE
IS_DIG_FALSE:
            STC
            CMC                 ; Clear Carry (CY = 0)
            RET
IS_DIG_TRUE:
            STC                 ; Set Carry (CY = 1)
            RET

; ------------------------------------------------------------------------------
; CHECK_KEYWORD_INT:
; Compares string in CUR_NAME buffer against "int\0".
; Output: Zero flag SET (Z=1) if match, CLEAR (Z=0) if mismatch.
; ------------------------------------------------------------------------------
CHECK_KEYWORD_INT:
            LXI D, CUR_NAME
            LDAX D
            CPI 69H             ; 'i'
            RNZ
            INX D
            LDAX D
            CPI 6EH             ; 'n'
            RNZ
            INX D
            LDAX D
            CPI 74H             ; 't'
            RNZ
            INX D
            LDAX D
            ORA A               ; '\0' (Null terminator)
            RET                 ; Z=1 if null terminator, Z=0 otherwise

; ==============================================================================
;                             END OF LEXER.ASM
; ==============================================================================
