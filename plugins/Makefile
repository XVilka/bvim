IDIR = ../include
CC = gcc
CFLAGS  = -I$(IDIR) -I../lua/include
DEPS = plugins.h
OBJ = test.o
LIBS = -lc -llua

plugins: test.so visualisation.so

test.so: test.o
	$(CC) -shared -fPIC -o $@ $^ $(CFLAGS) $(LIBS)

test.o: test.c
	$(CC) -c -fPIC -o $@ $< $(CFLAGS)

visualisation.so: visualisation.o
	$(CC) -shared -fPIC -o $@ $^ $(CFLAGS) $(LIBS) -lmgl

visualisation.o: visualisation.c
	$(CC) -c -fPIC -o $@ $< $(CFLAGS)

clean:
	rm *.o *.so 
