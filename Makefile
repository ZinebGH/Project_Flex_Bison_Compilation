# Makefile projet analyse syntaxique

# $@ : the current target
# $^ : the current prerequisites
# $< : the first current prerequisite

CC = gcc
CFLAGS = -Wall
LDFLAGS	= -Wall -ly -lfl
SRC = src
OBJ = obj
BIN = bin
EXEC = tpcc
HEADERS = $(SRC)/parser.h $(wildcard $(SRC)/*.h)
OBJECTS = $(OBJ)/lexer.o $(OBJ)/parser.o \
		$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(wildcard $(SRC)/*.c))

$(BIN)/$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(SRC)/lexer.c: $(SRC)/lexer.lex
	flex -o $@ $<

$(SRC)/parser.c $(SRC)/parser.h: $(SRC)/parser.y
	bison -d -o $(SRC)/parser.c $<

.PHONY: clean
clean:
	rm -f $(SRC)/lexer.c $(SRC)/parser.[ch] $(OBJ)/*.o