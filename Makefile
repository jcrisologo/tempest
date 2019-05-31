IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR) -O2

ODIR=obj
LDIR =../lib

LIBS=-lm -lSDL2 -lliquid -lconfig -L/usr/lib/i386-linux-gnu

_OBJ = pulse_shaper.o mixer.o util.o monitor_modulator.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

TARGETS = tempest_transmission pulse_shaper_test mixer_test

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean all

all: $(TARGETS)

$(TARGETS) : % : $(ODIR)/%.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(ODIR)/*.o $(TARGETS)
