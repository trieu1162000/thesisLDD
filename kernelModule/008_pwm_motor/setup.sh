#!/bin/bash
dtc pwm_rasp.dts -O dtb > pwm_rasp.dtbo
sudo cp pwm_rasp.dtbo /boot/overlays/