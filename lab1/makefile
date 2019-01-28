CC=gcc
CXX=g++
CFLAGS=-I.
CPPPREFLAGS=-Iinclude -Llib -g
CPPFLAGS=-lfreeglut -lopengl32 -lglu32

all: linije polygon

shade: shade.cpp
	$(CXX) $(CPPPREFLAGS) -o shade shade.cpp $(CPPFLAGS)

cheat: cheat.cpp
	$(CXX) $(CPPPREFLAGS) -o cheat cheat.cpp $(CPPFLAGS)

path: path.o camera.o
	$(CXX) $(CPPPREFLAGS) -o path path.o camera.o $(CPPFLAGS)

path.o: path.cpp camera.h
	$(CXX) $(CPPPREFLAGS) -c path.cpp $(CPPFLAGS)

main: main.o camera.o
	$(CXX) $(CPPPREFLAGS) -o main main.o camera.o $(CPPFLAGS)

main.o: main.cpp camera.h
	$(CXX) $(CPPPREFLAGS) -c main.cpp $(CPPFLAGS)

camera.o: camera.cpp camera.h
	$(CXX) $(CPPPREFLAGS) -c camera.cpp $(CPPFLAGS)

.PHONY: clean

clean:
	del *.exe