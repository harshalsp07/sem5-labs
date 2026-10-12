%{
/*=================================================================
 * MiniC Lexical Analyzer (Flex / Lex Implementation)
 * Target: Compiler Design Laboratory
 * 
 * Tokenizes the MiniC Language and simulates the Intel 8085
 * 2-byte Token Memory Layout (Type, Attribute) starting at 2800H.
 *=================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- Token Type Codes (matching 8085 assembly specification) --- */
#define TOK_EOF     0   /* End of input sentinel (00H) */
#define TOK_INT     1   /* 'int' keyword (01H) */
#define TOK_IDENT   2   /* Identifier [a-zA-Z_][a-zA-Z0-9_]* (02H) */
#define TOK_NUM     3   /* Numeric literal 0..255 (03H) */
#define TOK_PLUS    4   /* '+' operator (04H) */
#define TOK_MINUS   5   /* '-' operator (05H) */
#define TOK_STAR    6   /* '*' operator (06H) */
#define TOK_SLASH   7   /* '/' operator (07H) */
#define TOK_EQUAL   8   /* '=' assignment (08H) */
#define TOK_SEMI    9   /* ';' statement terminator (09H) */

/* Maximum tokens stored in memory simulation */
#define MAX_TOKENS 1024
#define MEM_BASE 0x2800

/* Token Record structure matching 8085 (Type Byte, Value Byte) */
typedef struct {
    int line;
    int token_id;
    char token_name[20];
    char lexeme[64];
    unsigned char val_byte;  /* 8-bit attribute stored in 8085 memory */
    unsigned short mem_addr; /* Simulated 8085 memory address (2800H+) */
} TokenRecord;

static TokenRecord token_list[MAX_TOKENS];
static int token_count = 0;
static int current_line = 1;

/* Helper to record token */
void record_token(int id, const char *name, const char *lex, unsigned char val) {
    if (token_count < MAX_TOKENS) {
        token_list[token_count].line = current_line;
        token_list[token_count].token_id = id;
        strncpy(token_list[token_count].token_name, name, 19);
        strncpy(token_list[token_count].lexeme, lex, 63);
        token_list[token_count].val_byte = val;
        token_list[token_count].mem_addr = MEM_BASE + (token_count * 2);
        token_count++;
    }
}

%}

%option noyywrap
%option nounput
%option noinput

/* --- Regular Definitions --- */
DIGIT       [0-9]
NUMBER      {DIGIT}+
LETTER      [a-zA-Z_]
IDENT       {LETTER}[a-zA-Z0-9_]*
WS          [ \t\r]+
COMMENT_HASH  #[^\r\n]*
COMMENT_SLASH "//"[^\r\n]*

%%

\n              { current_line++; }
{WS}            { /* Skip whitespace */ }
{COMMENT_HASH}  { /* Skip hash comments */ }
{COMMENT_SLASH} { /* Skip slash comments */ }

"int"           { record_token(TOK_INT,   "TOK_INT",   yytext, 0x00); return TOK_INT; }
"+"             { record_token(TOK_PLUS,  "TOK_PLUS",  yytext, 0x00); return TOK_PLUS; }
"-"             { record_token(TOK_MINUS, "TOK_MINUS", yytext, 0x00); return TOK_MINUS; }
"*"             { record_token(TOK_STAR,  "TOK_STAR",  yytext, 0x00); return TOK_STAR; }
"/"             { record_token(TOK_SLASH, "TOK_SLASH", yytext, 0x00); return TOK_SLASH; }
"="             { record_token(TOK_EQUAL, "TOK_EQUAL", yytext, 0x00); return TOK_EQUAL; }
";"             { record_token(TOK_SEMI,  "TOK_SEMI",  yytext, 0x00); return TOK_SEMI; }

{NUMBER}        {
                    int val = atoi(yytext);
                    if (val > 255) {
                        fprintf(stderr, "[Warning] Line %d: Number %d overflows 8-bit unsigned range (0..255). Truncating to %d.\n",
                                current_line, val, val & 0xFF);
                    }
                    unsigned char uval = (unsigned char)(val & 0xFF);
                    record_token(TOK_NUM, "TOK_NUM", yytext, uval);
                    return TOK_NUM;
                }

{IDENT}         {
                    /* Value byte stores ASCII code of first character */
                    unsigned char first_char = (unsigned char)yytext[0];
                    record_token(TOK_IDENT, "TOK_IDENT", yytext, first_char);
                    return TOK_IDENT;
                }

.               {
                    fprintf(stderr, "[Lexer Error] Line %d: Unrecognized character '%c' (0x%02X)\n",
                            current_line, yytext[0], (unsigned char)yytext[0]);
                }

<<EOF>>         {
                    record_token(TOK_EOF, "TOK_EOF", "$/EOF", 0x00);
                    return 0;
                }

%%

/* --- Diagnostic and Memory Simulation Driver --- */
void print_token_table() {
    printf("\n========================================================================================\n");
    printf("                                MINIC TOKEN STREAM TABLE                                \n");
    printf("========================================================================================\n");
    printf("%-6s | %-6s | %-12s | %-16s | %-18s | %-12s\n",
           "Index", "Line", "Token ID", "Token Name", "Lexeme", "Value Byte");
    printf("----------------------------------------------------------------------------------------\n");

    for (int i = 0; i < token_count; i++) {
        TokenRecord *t = &token_list[i];
        char val_str[32];
        if (t->token_id == TOK_NUM) {
            snprintf(val_str, sizeof(val_str), "0x%02X (%u)", t->val_byte, t->val_byte);
        } else if (t->token_id == TOK_IDENT) {
            snprintf(val_str, sizeof(val_str), "0x%02X ('%c')", t->val_byte, t->val_byte);
        } else {
            snprintf(val_str, sizeof(val_str), "0x%02X (None)", t->val_byte);
        }

        printf("#%-5d | Line %-2d | 0x%02X (%d)   | %-12s | %-16s | %-18s\n",
               i, t->line, t->token_id, t->token_id, t->token_name, t->lexeme, val_str);
    }
    printf("========================================================================================\n");
    printf(" Total Tokens Extracted: %d\n", token_count - 1); /* Excluding EOF */
}

void print_memory_dump() {
    printf("\n========================================================================================\n");
    printf("                    SIMULATED INTEL 8085 TOKEN MEMORY DUMP (2800H+)                     \n");
    printf("                 [Each Token = 2 Bytes: Byte 0 = Type, Byte 1 = Value]                  \n");
    printf("========================================================================================\n");
    printf(" Address : Type Val | Disassembly / Interpretation\n");
    printf("---------:----------:-------------------------------------------------------------------\n");

    for (int i = 0; i < token_count; i++) {
        TokenRecord *t = &token_list[i];
        printf(" 0x%04X  :  %02X   %02X  | %-10s -> %-12s (Val: 0x%02X)\n",
               t->mem_addr, t->token_id, t->val_byte, t->token_name, t->lexeme, t->val_byte);
    }
    printf("========================================================================================\n");

    printf("\n--- Flat Hex Dump for Sim8085 Memory View (2800H) ---\n");
    for (int i = 0; i < token_count; i++) {
        TokenRecord *t = &token_list[i];
        if (i % 8 == 0) {
            if (i > 0) printf("\n");
            printf("28%02X: ", i * 2);
        }
        printf("%02X %02X ", t->token_id, t->val_byte);
    }
    printf("\n========================================================================================\n\n");
}

int main(int argc, char **argv) {
    printf("******************************************************************\n");
    printf("*            MiniC Lexical Analyzer (Flex + 8085 Memory)         *\n");
    printf("******************************************************************\n");

    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            perror("Error opening input file");
            return 1;
        }
        printf("Reading MiniC source code from file: %s\n", argv[1]);
        yyin = f;
    } else {
        printf("No input file provided. Using default MiniC reference test program...\n\n");
        const char *default_code =
            "// Reference MiniC Program\n"
            "int a;\n"
            "int b = 5;\n"
            "int c = a * b + 3;\n"
            "int d;\n"
            "d = c - b * 2;\n";
        printf("--- Source Program ---\n%s\n----------------------\n", default_code);
        yy_scan_string(default_code);
    }

    /* Execute scanner */
    while (yylex() != 0);

    /* Output formatted analysis */
    print_token_table();
    print_memory_dump();

    return 0;
}
