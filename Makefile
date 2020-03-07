SHELL=C:/Windows/System32/cmd.exe
CC=C:\Libraries\emscripten\emscripten\1.37.1\em++.bat
CFLAGS=-O3 -Wno-tautological-constant-out-of-range-compare -std=c++11
LDFLAGS=-O3 --llvm-lto 1 -s DISABLE_EXCEPTION_CATCHING=0 
INCLUDES=-Isource/json -Isrc -Isource/CImg
SOURCES=$(wildcard src/*.cpp) 
OBJECTS=$(SOURCES:.cpp=.o)

all:emscripten/BOSS.js

JSFLAGS=--memory-init-file 0 -s EXPORTED_FUNCTIONS="['_BOSS_JS_Init', '_BOSS_JS_GetBuildOrderPlot']" --preload-file bin/

emscripten/BOSS.js:$(OBJECTS) Makefile
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(JSFLAGS)

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm $(OBJECTS)
