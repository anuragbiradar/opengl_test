CC=gcc
CPP=g++
CFLAGS=-std=c++11 -g
# Enable below line to disable normalization of normal vector
#CFLAGS+=-DNO_NORMALIZE
HEADER=-I./inc
LOADER=-lGL -lGLEW -lglfw
BIN=
%.o: %.cpp
	$(CPP) -c -o $@ $< $(HEADER) $(CFLAGS)

assign: src/ply_parser.o src/render.o src/main.o
	mkdir -p bin
	$(CPP) -o bin/assign src/*.o $(LOADER)
	cp data/* bin/

clean:
	rm -rf bin/*
	rm -rf src/*.o
	rm -rf data/\.*.swp
	rm -rf src/\.*.swp
	rm -rf inc/\.*.swp
