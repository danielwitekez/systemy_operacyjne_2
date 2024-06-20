obj-m := genetlink.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

default: genetlink.c gl
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

gl: gl.c
	gcc -Wall -o gl gl.c $(shell pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)

distclean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f gl
