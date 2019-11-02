INCLUDE=-I../include
LINK=-lpthread -lm

CC=g++
CFLAGS=-g -ansi -Wall -O3 -std=c++17 $(INCLUDE) -funroll-loops

%: %.cpp
	$(CC) $(CFLAGS) -o $@ $< $(LINK)
