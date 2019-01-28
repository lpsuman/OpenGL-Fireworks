CC=gcc
CXX=g++
CFLAGS=-I.
CPPPREFLAGS=-Iinclude -Llib -g
CPPFLAGS=-lfreeglut -lopengl32 -lglu32

all: particle

solution: solution.cpp
	$(CXX) $(CPPPREFLAGS) -o solution solution.cpp $(CPPFLAGS)

particle: particle.o spline.o control.o
	$(CXX) $(CPPPREFLAGS) -o particle particle.o spline.o control.o $(CPPFLAGS)

# particle: particle.o spline.o control.o
# 	$(CXX) $(CPPPREFLAGS) -o particle particle.o spline.o control.o -lfreeglut include/SOIL/lib/libSOIL.lib -lopengl32 -lglu32

particle.o: particle.cpp particle.h spline.h control.h
	$(CXX) $(CPPPREFLAGS) -c particle.cpp $(CPPFLAGS)

spline.o: spline.cpp spline.h
	$(CXX) $(CPPPREFLAGS) -c spline.cpp $(CPPFLAGS)

camera.o: camera.cpp camera.h
	$(CXX) $(CPPPREFLAGS) -c camera.cpp $(CPPFLAGS)

control.o: control.cpp control.h particle.h
	$(CXX) $(CPPPREFLAGS) -c control.cpp $(CPPFLAGS)

.PHONY: clean

clean:
	del *.exe *.o