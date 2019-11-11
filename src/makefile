INCLUDE=-I../include
LINK=-lpthread -lm

CC=g++
CFLAGS=-Wall -fPIC -O3 -std=c++17 -shared $(INCLUDE)

all: libfractal

libfractal: ../include/fractal.h fractal.cpp
	$(CC) $(CFLAGS) -o ../lib/libfractal.so fractal.cpp $(LINK)
