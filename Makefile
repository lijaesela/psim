EXE = psim
SRC = main.c ui.c ui.h config.h
FLAGS = -Wall -Wextra -pedantic -std=c11
LIBS = -lraylib

$(EXE): $(SRC)
	$(CC) -o $(EXE) $(SRC) $(CFLAGS) $(LIBS)

.PHONY: run
run: $(EXE)
	./$(EXE)
