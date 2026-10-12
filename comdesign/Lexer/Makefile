# Makefile for MiniC Lexical Analyzer

CC = gcc
CFLAGS = -Wall -Wextra -O2
FLEX = flex

TARGET = lexer_flex
LEX_SRC = lexer.l
GEN_C = lex.yy.c

all: $(TARGET)

$(TARGET): $(GEN_C)
	$(CC) $(CFLAGS) -o $(TARGET) $(GEN_C)

$(GEN_C): $(LEX_SRC)
	$(FLEX) $(LEX_SRC)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@echo "=== TEST 1: Reference Program ==="
	./$(TARGET) sample1.mc
	@echo "\n=== TEST 2: Multi-digit and Arithmetic ==="
	./$(TARGET) sample2.mc
	@echo "\n=== TEST 3: Reassignment and Comments ==="
	./$(TARGET) sample3.mc

clean:
	rm -f $(TARGET) $(GEN_C)

.PHONY: all run test clean
