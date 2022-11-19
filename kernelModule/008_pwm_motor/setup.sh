#!/bin/bash
dtc pwm_motor_driver_rasp.dts -O dtb > pwm_motor_driver_rasp.dtbo
sudo cp pwm_motor_driver_rasp.dtbo /boot/overlays/
