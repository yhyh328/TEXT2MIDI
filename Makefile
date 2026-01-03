# ./text2midi.exe song.txt [song.mid]

CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=c11

SRC=text2midi.c
OUT=text2midi.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)

