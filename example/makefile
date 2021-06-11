INCLUDE=-I../include
LINK=-lpthread -lm -lfractal

CC=g++
CFLAGS=-Wall -O3 -std=c++17 $(INCLUDE)

%: %.cpp ../include ../lib
	$(CC) $(CFLAGS) -o $@ $< $(LINK)
