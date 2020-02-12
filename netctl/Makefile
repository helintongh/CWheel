obj-m += netcco.o

all: netctl
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm netctl
	
netctl:	netctl.c nc_def.h
