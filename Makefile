EXE = psim
SRC = main.c ui.c ui.h config.h
FLAGS = -Wall -Wextra -std=c11 -pedantic -ggdb
LIBS = -lraylib

$(EXE): $(SRC)
	$(CC) -o $(EXE) $(SRC) $(FLAGS) $(LIBS)

.PHONY: run
run: $(EXE)
	./$(EXE)
