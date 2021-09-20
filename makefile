CC=gcc
CFLAGS=-c -Wall
SOURCES=main.c commands.c program.c
OBJECTS=$(SOURCES:.c=.o)
EXEC=editor

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJECTS) $(EXEC)

valgrind: $(EXEC)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$<