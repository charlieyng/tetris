EXE=Tetris
OBJECTS=main.o tetris.o
CC=gcc
FLAGS=-g -I./ -O3

$(EXE): $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) -o $(EXE)

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@
