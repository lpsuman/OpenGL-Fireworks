CXX=g++
CPPPREFLAGS=-Iinclude -Llib -g
CPPFLAGS=-lfreeglut -lopengl32 -lglu32

ifeq ($(OS),Windows_NT)
	MAIN_OUT := main.exe
	RM := del
else
	MAIN_OUT := main.out
	RM := rm
endif

all: $(MAIN_OUT)

# $@: the target filename.
# $*: the target filename without the file extension.
# $<: the first prerequisite filename.
# $^: the filenames of all the prerequisites, separated by spaces, discard duplicates.
# $+: similar to $^, but includes duplicates.
# $?: the names of all prerequisites that are newer than the target, separated by spaces.

$(MAIN_OUT): main.o control.o firework.o fuse.o particle.o spline.o imageloader.o terrain.o
	$(CXX) $(CPPPREFLAGS) -o $@ $^ $(CPPFLAGS)

main.o: main.cpp control.h firework.h fuse.h particle.h spline.h imageloader.h terrain.h
	$(CXX) $(CPPPREFLAGS) -c $*.cpp $(CPPFLAGS)

control.o: control.cpp control.h firework.h fuse.h particle.h
	$(CXX) $(CPPPREFLAGS) -c $*.cpp $(CPPFLAGS)

firework.o: firework.cpp firework.h particle.h
	$(CXX) $(CPPPREFLAGS) -c $*.cpp $(CPPFLAGS)

fuse.o: fuse.cpp fuse.h particle.h spline.h
	$(CXX) $(CPPPREFLAGS) -c $*.cpp $(CPPFLAGS)

particle.o: particle.cpp particle.h
	$(CXX) $(CPPPREFLAGS) -c $*.cpp $(CPPFLAGS)

spline.o: spline.cpp spline.h
	$(CXX) $(CPPPREFLAGS) -c $*.cpp $(CPPFLAGS)

imageloader.o: imageloader.cpp imageloader.h
	$(CXX) $(CPPPREFLAGS) -c $*.cpp $(CPPFLAGS)

terrain.o: terrain.cpp terrain.h imageloader.h
	$(CXX) $(CPPPREFLAGS) -c $*.cpp $(CPPFLAGS)

.PHONY: clean

clean:
	del *.exe *.o