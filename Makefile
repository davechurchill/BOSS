ifeq ($(TARGET),js)
  CC = emcc
else
  CC = g++
endif

CFLAGS=-O3 -std=c++17 -flto -Wformat=0
LDFLAGS=-O3 -flto -pthread
LDFLAGS_SFML=-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -s DISABLE_EXCEPTION_CATCHING=0 
JSFLAGS=--memory-init-file 0 -s EXPORTED_FUNCTIONS="['_BOSS_JS_Init', '_BOSS_JS_GetBuildOrderPlot']" -s EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap"] --preload-file bin/

INCLUDES=-Isrc -Isrc/BOSS -Isrc/json -Isrc/search -Isrc/test -Isrc/sfml

SRC_BOSS=$(wildcard src/BOSS/*.cpp src/search/*.cpp) 
OBJ_BOSS=$(SRC_BOSS:.cpp=.o)

SRC_EXPERIMENTS=$(wildcard src/experiments/*.cpp) 
OBJ_EXPERIMENTS=$(SRC_EXPERIMENTS:.cpp=.o)

SRC_SFML=$(wildcard src/sfml/*.cpp) 
OBJ_SFML=$(SRC_SFML:.cpp=.o)

SRC_TEST=$(wildcard src/test/*.cpp src/test/catch2/*.cpp) 
OBJ_TEST=$(SRC_TEST:.cpp=.o)

SRC_EMSCRIPTEN=$(wildcard src/BOSS/*.cpp src/search/*.cpp src/emscripten/*.cpp) 
OBJ_EMSCRIPTEN=$(SRC_EMSCRIPTEN:.cpp=.o)

ifeq ($(TARGET),js)
  all: emscripten/BOSS.js
else
  all: bin/BOSS_Experiments bin/BOSS_SFML bin/BOSS_Test
endif

bin/BOSS_Experiments:$(OBJ_BOSS) $(OBJ_EXPERIMENTS) Makefile
	$(CC) $(OBJ_BOSS) $(OBJ_EXPERIMENTS) -o $@  $(LDFLAGS)

bin/BOSS_SFML:$(OBJ_BOSS) $(OBJ_SFML) Makefile
	$(CC) $(OBJ_BOSS) $(OBJ_SFML) -o $@  $(LDFLAGS) $(LDFLAGS_SFML)

bin/BOSS_Test:$(OBJ_BOSS) $(OBJ_TEST) Makefile
	$(CC) $(OBJ_BOSS) $(OBJ_TEST) -o $@  $(LDFLAGS)

emscripten/BOSS.js:$(OBJ_EMSCRIPTEN) Makefile
	$(CC) $(OBJ_EMSCRIPTEN) -o $@ $(LDFLAGS) $(JSFLAGS)

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@ 

clean:
	rm -f bin/BOSS_Experiments bin/BOSS_SFML bin/BOSS_Test emscripten/BOSS.js src/BOSS/*.o src/search/*.o src/experiments/*.o src/test/*.o src/sfml/*.o src/emscripten/*.o 

