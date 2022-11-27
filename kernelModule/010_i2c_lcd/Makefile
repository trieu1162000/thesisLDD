obj-m := lcd_i2c_driver.o
lcd_i2c_driver-y := lcd_i2c.o lcd_i2c_api.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean