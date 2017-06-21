obj-m += memory.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo insmod memory.ko

uninstall:
	sudo rmmod memory

mknod:
	sudo mknod /dev/memory c 60 0
	sudo chmod 666 /dev/memory
