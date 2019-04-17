IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -I/usr/include/SDL2 -O2

ODIR=obj
LDIR =../lib

LIBS=-lm -lSDL2 -L/usr/lib/i386-linux-gnu

_OBJ = rrcos.o pulse_shaper.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

TARGETS = tempest_transmission pulse_shaper_test

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean all

all: $(TARGETS)

$(TARGETS) : % : $(ODIR)/%.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(ODIR)/*.o 
