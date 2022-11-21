obj-m := rfid_rc522_driver.o
rfid_rc522_driver-objs := rc522_driver.o rc522_api.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean