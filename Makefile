# Variables
CC = gcc
CFLAGS =
# TARGETS = mul mod sqr spy
TARGETS = spy

# Default rule (runs when you just type 'make')
all: $(TARGETS)

# Rules for each executable
# mul: multiply_spy.c constants.h
# 	$(CC) $(CFLAGS) multiply_spy.c -o mul

# mod: modulo_spy.c constants.h
# 	$(CC) $(CFLAGS) modulo_spy.c -o mod

# sqr: sqr_spy.c constants.h
# 	$(CC) $(CFLAGS) sqr_spy.c -o sqr

spy: spy.cpp constants.h
	g++ -O0 spy.cpp -o spy

# Clean rule to remove binaries and output files
clean:
	rm -f $(TARGETS) *_output.txt