# !/bin/bash

sudo rmmod hcsr04_driver.ko
sudo rmmod lcd_i2c_driver.ko
sudo rmmod tcrt5000_driver.ko
sudo rmmod pwm_driver.ko
sudo rmmod direction_motor_driver.ko
sudo rmmod rfid_rc522_driver.ko
echo "Clean done."