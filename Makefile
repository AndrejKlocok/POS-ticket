# Compiler/Linker required information
CXX      	= gcc
CFLAGS		= -std=c11 -Wall -pedantic -O2 -pthread
TARGET		= proj1


all: 
	$(CXX)  $(CFLAGS) main.c -o $(TARGET)


clean:
	rm  $(TARGET)