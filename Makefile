CXX = gcc

CXXFLAGS = -Wall -c -O2
LDFLAGS = -L. -l6502 -O2 

all: lib6502.a test6502 testdecimal6502

lib6502.a: 6502.o
	ar rc lib6502.a 6502.o 

6502.o: 6502.c 6502.h
	$(CXX) $(CXXFLAGS) $< -o $@

test6502: test6502.o lib6502.a
	$(CXX) $< $(LDFLAGS) -o $@

test6502.o : test6502.c
	$(CXX) $(CXXFLAGS) $< -o $@

testdecimal6502 : testdecimal6502.o lib6502.a
	$(CXX) $< $(LDFLAGS) -o $@

testdecimal6502.o : testdecimal6502.c
	    $(CXX) $(CXXFLAGS) $< -o $@

clean: 
	rm *.o && rm -f test6502 && rm *.a && rm -f testdecimal6502
