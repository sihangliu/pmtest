ifndef $(NUM_CORES)
NUM_CORES	:= 1
endif

CC			:= -gcc
CXX			:= -g++-4.8
CFLAGS		:= -fPIC -std=c11 -DPMTEST_EXCLUDE -DNUM_CORES=$(NUM_CORES)
CXXFLAGS	:= -std=c++11 -fPIC -DPMTEST_EXCLUDE -DNUM_CORES=$(NUM_CORES) #-pedantic-errors -Wall -Wextra -Werror
LDFLAGS		:= -L/usr/lib -lstdc++ -lm -pthread #-lbacktrace -ldl
INCLUDE		:= -Iinclude/

BUILD		:= ./build
OBJ_DIR		:= $(BUILD)/objects
APP_DIR   	:= $(BUILD)/apps
LIB_DIR   	:= $(BUILD)/libs
KERNEL_DIR	:= /lib/modules/$(shell uname -r)/build

SRC_DIR		:= ./src

# set target name and corresponding objects for library creation, unit test, and kernel client
DYNAMICLINK_OBJECT		:= $(addprefix $(OBJ_DIR)/, pmtest.o common.o)

UNIT_TEST_TARGET		:= pmtest_test
UNIT_TEST_OBJECT		:= $(addprefix $(OBJ_DIR)/, main.o pmtest.o test.o common.o)

KERNEL_CLIENT_TARGET	:= pmtest_kernel_client
KERNEL_CLIENT_OBJECT	:= $(addprefix $(OBJ_DIR)/, kernel_client.o pmtest.o common.o)

all: build test kernel buildlib

nvml:
	cd nvml && ./build.sh

nvml-clean:
	cd nvml && ./clean.sh

nvml-install:
	cd nvml && make install

nvml-example:
	cd nvml/src/examples/libpmemobj/ && make

nvml-example-clean:
	cd nvml/src/examples/libpmemobj/ && make clean

nvml-exec:
	@ echo ./data_store $(arg) /mnt/pmem/pmtest 100
	@ rm -f /mnt/pmem/pmtest
	@ cd nvml/src/examples/libpmemobj/map/ && ./data_store $(arg) /mnt/pmem/pmtest 100

nvml-btree:
	@ ./patch/patch.sh . ./patch/real_bug/pmdk_btree.patch
	@ make nvml-example > /dev/null 2>&1
	@ make nvml-exec arg=btree
	@ ./patch/patch.sh -R . ./patch/real_bug/pmdk_btree.patch
	@ make nvml-example > /dev/null 2>&1

buildlib: $(DYNAMICLINK_OBJECT)
	@mkdir -p $(@D)
	ar -cvq $(LIB_DIR)/libpmtest.a $(DYNAMICLINK_OBJECT)
	$(CXX) -shared -o $(LIB_DIR)/libpmtest.so $(DYNAMICLINK_OBJECT)

test: $(APP_DIR)/$(UNIT_TEST_TARGET)

kernel: $(APP_DIR)/$(KERNEL_CLIENT_TARGET)

$(APP_DIR)/$(UNIT_TEST_TARGET): $(UNIT_TEST_OBJECT)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^ $(LDFLAGS)

$(APP_DIR)/$(KERNEL_CLIENT_TARGET): $(KERNEL_CLIENT_OBJECT)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LIB_DIR)

debug: CXXFLAGS += -DDEBUG -g -DDEBUG_FLAG=0
debug: all

release: CXXFLAGS += -O3
release: all

warning: CXXFLAGS += -O3 -DPMTEST_WARN -DDEBUG -g
warning: all

clean:
	-@rm -rf $(BUILD)

.PHONY: all build buildlib clean debug test kernel warning release nvml nvml-clean nvml-example nvml-example-clean nvml-btree
# g++-4.8 -std=c++11 -c common.cc -I../include -o common.o
# g++-4.8 -std=c++11 -c pmtest.cc -I../include -o pmtest.o
# gcc -c main.c -I../include -o main.o
# gcc common.o pmtest.o main.o -o main -L/usr/lib -lstdc++ -lm -pthread
