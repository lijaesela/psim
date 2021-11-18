EXE = psim

C = main.c ui.c
H = ui.h config.h

FLAGS = -Wall -Wextra -std=c11 -pedantic -ggdb
LIBS = -lraylib

# keep these if you must use a downloaded raylib from:
# https://github.com/raysan5/raylib/releases
IPATH = -Iraylib-4.0.0_linux_amd64/include
LPATH = -Lraylib-4.0.0_linux_amd64/lib

$(EXE): $(C) $(H)
	$(CC) -o $(EXE) $(C) $(FLAGS) $(IPATH) $(LPATH) $(LIBS)

.PHONY: run
run: $(EXE)
	./$(EXE)
