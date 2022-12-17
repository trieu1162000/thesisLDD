# !/bin/bash

sudo insmod hcsr04_obstacle_driver.ko
sudo insmod hcsr04_water_capacity_driver.ko
sudo insmod lcd_i2c_driver.ko
sudo insmod tcrt5000_driver.ko
sudo insmod pwm_driver.ko
sudo insmod direction_motor_driver.ko
sudo insmod rfid_rc522_driver.ko
sudo insmod ina219_driver.ko
sudo insmod encoder_driver.ko

dtc thesis_rasp.dts -O dtb > thesis_rasp.dtbo
sudo cp thesis_rasp.dtbo /boot/overlays/

echo "Insmod done."
