KDIR := /lib/modules/$(shell uname -r)/build

obj-m += test_socket.o

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc -o nvmveri_socket.o -c nvmveri_socket.c -I../include
	g++ -o nvmveri_socket nvmveri_socket.o ../build/objects/src/nvmveri.o ../build/objects/src/common.o -pthread
debug:
	gcc -o nvmveri_socket.o -c nvmveri_socket.c -I../include -g
	g++ -o nvmveri_socket nvmveri_socket.o ../build/objects/src/nvmveri.o ../build/objects/src/common.o -pthread -g

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
