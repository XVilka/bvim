IDIR = ../include
CC = gcc
CFLAGS  = -I$(IDIR)
DEPS = plugins.h
OBJ = test.o

%.o: %.c $(DEPS)
	$(CC) -c -fPIC -o $@ $< $(CFLAGS)

test.so: test.o
	gcc -shared -o $@ $^ $(CFLAGS) -lc

