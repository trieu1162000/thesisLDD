obj-m := lcd_i2c_driver.o
lcd_i2c_driver-y := lcd_i2c.o lcd_i2c_api.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
dts:
	dtc lcd_i2c_rasp.dts -O dtb > lcd_i2c_rasp.dtbo
	sudo cp lcd_i2c_rasp.dtbo /boot/overlays/

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
