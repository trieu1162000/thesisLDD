# !/bin/bash

sudo rmmod hcsr04_obstacle_driver.ko
sudo rmmod hcsr04_water_capacity_driver.ko
sudo rmmod lcd_i2c_driver.ko
sudo rmmod tcrt5000_driver.ko
sudo rmmod pwm_driver.ko
sudo rmmod direction_motor_driver.ko
sudo rmmod rfid_rc522_driver.ko
sudo rmmod ina219_driver.ko
sudo rmmod encoder_driver.ko

rm -f thesis_rasp.dtbo

echo "Clean done."
