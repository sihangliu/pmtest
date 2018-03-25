CC        := -gcc
CXX       := -g++-4.8
CFLAGS    := -fPIC
CXXFLAGS  := -std=c++11 -fPIC#-pedantic-errors -Wall -Wextra -Werror
LDFLAGS   := -L/usr/lib -lstdc++ -lm -pthread
BUILD     := ./build
OBJ_DIR   := $(BUILD)/objects
APP_DIR   := $(BUILD)/apps
LIB_DIR   := $(BUILD)/libs
TARGET    := nvmveri
INCLUDE   := -Iinclude/
SRC       := $(wildcard src/*.cc)
C_SRC     := $(wildcard src/*.c)

OBJECTS   := $(SRC:%.cc=$(OBJ_DIR)/%.o)
C_OBJECTS := $(SRC:%.c=$(OBJ_DIR)/%.o)

all: build $(APP_DIR)/$(TARGET) buildlib

buildlib: $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) -shared -o $(LIB_DIR)/libnvmveri.so $(<D)/nvmveri.o


$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) -o $(APP_DIR)/$(TARGET) $(OBJECTS)

cc: build $(OBJECTS) $(C_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(INCLUDE) -o $(APP_DIR)/$(TARGET) $(OBJECTS) $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<


.PHONY: all build buildlib clean debug release

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LIB_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rf $(OBJ_DIR)/*
	-@rm -rf $(APP_DIR)/*
	-@rm -rf $(LIB_DIR)/*

# g++-4.8 -std=c++11 -c common.cc -I../include -o common.o
# g++-4.8 -std=c++11 -c nvmveri.cc -I../include -o nvmveri.o
# gcc -c main.c -I../include -o main.o
# gcc common.o nvmveri.o main.o -o main -L/usr/lib -lstdc++ -lm -pthread
