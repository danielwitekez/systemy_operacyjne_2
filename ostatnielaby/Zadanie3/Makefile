obj-m := netlink.o
KERNELDIR = /lib/modules/$(shell uname -r)/build

default: netlink.c  nl
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

nl: nl.c
	gcc -Wall -o nl nl.c $(shell pkg-config --cflags --libs libnl-3.0)

distclean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f nl
