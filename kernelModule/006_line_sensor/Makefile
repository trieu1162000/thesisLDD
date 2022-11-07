obj-m := qtr_5rc_driver.o
qtr_5rc_driver-objs := qtr_5rc.o qtr_5rc_api.o

HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build/

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean