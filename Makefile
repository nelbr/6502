CXX = gcc

CXXFLAGS = -Wall -c 
LDFLAGS = -L. -l6502

all: lib6502.a test6502

lib6502.a: 6502.o
	ar rc lib6502.a 6502.o 

6502.o: 6502.c
	$(CXX) $(CXXFLAGS) $< -o $@

test6502: test6502.o
	$(CXX) $< $(LDFLAGS) -o $@

test6502.o : test6502.c
	$(CXX) $(CXXFLAGS) $< -o $@

clean: 
	rm *.o && rm -f test6502 && rm *.a
