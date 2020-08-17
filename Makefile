#paths
INCLUDE = ./include
SRC = ./src

#compiler
CC = gcc

#compile options
CFLAGS = -Wall -g -I$(INCLUDE)

OBJS = $(SRC)/main.o $(SRC)/semaphores.o $(SRC)/shared_memory.o

EXEC = ex

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC)