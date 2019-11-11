INCLUDE=-I../include
LIB=-L../lib
LINK=-lpthread -lm -lfractal

CC=g++
CFLAGS=-Wall -O3 -std=c++17 $(INCLUDE) $(LIB)

%: %.cpp ../include ../lib
	$(CC) $(CFLAGS) -o $@ $< $(LINK)
