#! /bin/bash

echo "build .dtbo"
dtc rfid_rasp.dts -O dtb > rfid_rasp.dtbo
echo "copy dtbo to /boot/overlays/"
sudo cp rfid_rasp.dtbo /boot/overlays/
echo "you must update /boot/config.txt for active overlay as step below:"
echo "1. sudo vim /boot/config.txt
2. add new line: dtoverlay=rfid_rasp"
