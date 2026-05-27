obj-m += pcd_device_setup.o
obj-m += pcd_driver_setup.o

KERN_DIR_HOST = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERN_DIR_HOST) M=$(PWD) modules

clean:
	make -C $(KERN_DIR_HOST) M=$(PWD) clean
