CFLAG = -O2 -march=native -fno-strict-aliasing
CC = gcc
BIN = test-no-amx
CFILES =test-no-amx.c

all:
	$(CC) $(CFLAG) $(CFILES) -o $(BIN) $(LIBS)

clean:
	-rm $(BIN)

.PHONY: clean

