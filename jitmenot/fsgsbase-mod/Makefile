obj-m += setfsgsbase.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

start: all
	sudo insmod ./setfsgsbase.ko
	sudo rmmod setfsgsbase
	dmesg | tail
