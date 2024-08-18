CC = gcc
CFLAGS = -I"C:\msys64\mingw32\include" -L"C:\msys64\mingw32\lib" -I"C:\Program Files\MySQL\MySQL Server 8.0\include" -L"C:\msys64\mingw32\lib"
LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer

all:

%: %.c functions.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(wildcard *.exe)

