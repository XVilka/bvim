IDIR = ../include
CC = gcc
CFLAGS  = -I$(IDIR)
DEPS = plugins.h
OBJ = test.o

plugins: test.so

test.so: test.o
	gcc -shared -fPIC -o $@ $^ $(CFLAGS) -lc

test.o: test.c
	$(CC) -c -fPIC -o $@ $< $(CFLAGS)

clean:
	rm *.o test.so
