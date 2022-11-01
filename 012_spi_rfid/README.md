To work with SPI on BBB, you must hard code on device tree or use device tree overlay but i recommend using dto bcs its easy to me. Hmm, having many difficult and taking a lot of time but its work ;)

Step 1: Copy folder overlayRFID to BBB and run ./build_dts_overlay.sh 

Step 2: Modify some lines in /boot/uEnv.txt and reboot
+ Add this: uboot_overlay_addr4=/lib/firmware/BB-RFID-00A0.dtbo
+ Comment this: #enable_uboot_cape_universal=1

Step 3: Run make (in host) to cross compile kernel in this folder (012.../)

Step 4: Run make copy-drv to scp file .ko to BBB

Step 5: Load module and use rc522App in folder 012.../testCodeApps (you must build it by gcc before using) to test the module which have been loaded to the kernel on BBB
