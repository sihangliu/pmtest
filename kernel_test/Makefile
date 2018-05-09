KDIR := /lib/modules/$(shell uname -r)/build

obj-m += test_socket.o

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(shell pwd) modules
	gcc -o nvmveri_client.o -c nvmveri_client.c -I../include
	g++ -o nvmveri_client nvmveri_client.o ../build/objects/src/nvmveri.o ../build/objects/src/common.o -pthread
debug:
	gcc -o nvmveri_client.o -c nvmveri_client.c -I../include -g
	g++ -o nvmveri_client nvmveri_client.o ../build/objects/src/nvmveri.o ../build/objects/src/common.o -pthread -g

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*
		make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
