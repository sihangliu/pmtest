ifndef $(NUM_CORES)
NUM_CORES	:= 1
endif

CC			:= -gcc
CXX			:= -g++-4.8
CFLAGS		:= -fPIC -std=c11 -DNUM_CORES=$(NUM_CORES)
CXXFLAGS	:= -std=c++11 -fPIC -DNVMVERI_EXCLUDE -DNUM_CORES=$(NUM_CORES) #-pedantic-errors -Wall -Wextra -Werror
LDFLAGS		:= -L/usr/lib -lstdc++ -lm -pthread
INCLUDE		:= -Iinclude/

BUILD		:= ./build
OBJ_DIR		:= $(BUILD)/objects
APP_DIR   	:= $(BUILD)/apps
LIB_DIR   	:= $(BUILD)/libs
KERNEL_DIR	:= /lib/modules/$(shell uname -r)/build

SRC_DIR		:= ./src

# set target name and corresponding objects for library creation, unit test, and kernel client
DYNAMICLINK_OBJECT		:= $(addprefix $(OBJ_DIR)/, nvmveri.o common.o)

UNIT_TEST_TARGET		:= nvmveri_test
UNIT_TEST_OBJECT		:= $(addprefix $(OBJ_DIR)/, main.o nvmveri.o test.o common.o)

KERNEL_CLIENT_TARGET	:= nvmveri_kernel_client
KERNEL_CLIENT_OBJECT	:= $(addprefix $(OBJ_DIR)/, kernel_client.o nvmveri.o common.o)

all: build $(APP_DIR)/$(UNIT_TEST_TARGET) $(APP_DIR)/$(KERNEL_CLIENT_TARGET) buildlib

buildlib: $(DYNAMICLINK_OBJECT)
	@mkdir -p $(@D)
	ar -cvq $(LIB_DIR)/libnvmveri.a $(DYNAMICLINK_OBJECT)
	$(CXX) -shared -o $(LIB_DIR)/libnvmveri.so $(DYNAMICLINK_OBJECT)

test: $(APP_DIR)/$(UNIT_TEST_TARGET)

kernel: $(APP_DIR)/$(KERNEL_CLIENT_TARGET)

$(APP_DIR)/$(UNIT_TEST_TARGET): $(UNIT_TEST_OBJECT)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) -o $@ $^

$(APP_DIR)/$(KERNEL_CLIENT_TARGET): $(KERNEL_CLIENT_OBJECT)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LIB_DIR)

debug: CXXFLAGS += -DDEBUG -DDEBUG_FLAG=0 -g
debug: all

release: CXXFLAGS += -O3
release: all

warning: CXXFLAGS += -O3 -DNVMVERI_WARN -DDEBUG -DDEBUG_FLAG=0 -g
warning: all

exclude: CXXFLAGS += -O3 -DNVMVERI_EXCLUDE -DDEBUG -DDEBUG_FLAG=0 -g
exclude: all

clean:
	-@rm -rf $(BUILD)

.PHONY: all build buildlib clean debug kernel release
# g++-4.8 -std=c++11 -c common.cc -I../include -o common.o
# g++-4.8 -std=c++11 -c nvmveri.cc -I../include -o nvmveri.o
# gcc -c main.c -I../include -o main.o
# gcc common.o nvmveri.o main.o -o main -L/usr/lib -lstdc++ -lm -pthread
