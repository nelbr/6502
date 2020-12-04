CXX = gcc

CXXFLAGS = -Wall -c -O2
LDFLAGS = -L. -l6502 -O2

all: lib6502.a test6502

lib6502.a: 6502.o
	ar rc lib6502.a 6502.o 

6502.o: 6502.c
	$(CXX) $(CXXFLAGS) $< -o $@

test6502: test6502.o lib6502.a
	$(CXX) $< $(LDFLAGS) -o $@

test6502.o : test6502.c
	$(CXX) $(CXXFLAGS) $< -o $@

clean: 
	rm *.o && rm -f test6502 && rm *.a
