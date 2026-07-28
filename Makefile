
CC = gcc
CFLAGS =
TARGETS = spy spy_triggered

all: $(TARGETS)

spy: spy2.c constants.h symbol_resolver.h
	gcc -m32 -O0 spy2.c -o spy

spy_triggered: spy_triggered.cpp constants.h symbol_resolver.h
	g++ -m32 -O0 spy_triggered.cpp -o spy_triggered

clean:
	rm -f $(TARGETS) outputs/*

.PHONY: all clean
