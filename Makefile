IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -I/usr/include/SDL2 -O2

ODIR=obj
LDIR =../lib

LIBS=-lm -lSDL2 -L/usr/lib/i386-linux-gnu

_OBJ = player.o rrcos.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean all

all: tempest_transmission

tempest_transmission: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(ODIR)/*.o 
