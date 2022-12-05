# !/bin/bash

sudo insmod hcsr04_driver.ko
sudo insmod lcd_i2c_driver.ko
sudo insmod tcrt5000_driver.ko
sudo insmod pwm_driver.ko
sudo insmod direction_motor_driver.ko
sudo insmod rfid_rc522_driver.ko
echo "Insmod done."
