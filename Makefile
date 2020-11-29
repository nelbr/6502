CXX = gcc
# Update these paths to match your installation
# You may also need to update the linker option rpath, which sets where to look for
# the SDL2 libraries at runtime to match your install
# SDL_LIB = -L/usr/local/lib -lSDL2 -lSDL2_ttf -Wl,-rpath=/usr/local/lib
# SDL_INCLUDE = -I/usr/local/include

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
