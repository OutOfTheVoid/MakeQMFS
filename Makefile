CC=gcc
CXX=g++
LD=g++

LINK_FLAGS= -O2
C_FLAGS = -c -Wall -Wextra $(LINK_FLAGS) -Iinclude
CPP_FLAGS= $(C_FLAGS)

BIN=mkqmfs

all: build test
	
build: bin/$(BIN)
	
test: FORCE
	./bin/$(BIN) -dir test/in -out test/out/initrd.img -rootname initrd
	
FORCE:
	
OBJS= obj/Main.o obj/HostFile.o obj/Directory.o obj/QMFS.o

bin/$(BIN): $(OBJS)
	$(LD) $(OBJS) $(LINK_FLAGS) -o bin/$(BIN)

obj/Main.o: src/Main.cpp include/HostFile.h
	$(CXX) src/Main.cpp $(CPP_FLAGS) -o obj/Main.o -DBIN_NAME=\"$(BIN)\"
	
obj/HostFile.o: src/HostFile.cpp include/HostFile.h
	$(CXX) src/HostFile.cpp $(CPP_FLAGS) -o obj/HostFile.o
	
obj/Directory.o: src/Directory.cpp include/Directory.h
	$(CXX) src/Directory.cpp $(CPP_FLAGS) -o obj/Directory.o
	
obj/QMFS.o: src/QMFS.cpp include/QMFS.h include/Directory.h include/HostFile.h
	$(CXX) src/QMFS.cpp $(CPP_FLAGS) -o obj/QMFS.o
	
clean:
	rm -r obj
	mkdir obj

