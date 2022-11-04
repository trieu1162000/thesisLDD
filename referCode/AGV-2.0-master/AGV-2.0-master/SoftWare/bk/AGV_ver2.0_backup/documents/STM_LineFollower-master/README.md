# STM_LineFollower

## Overview 
STM32 based Line Follower. Autonomous robot designed to complete specified tracks without any human intervention.

## Description 
The vital component required for project's functioning is STM32f407vg microcontroller. Board's power supply is 2600mAh power bank connected to USB port. Whole platform is put into motion by two DAGU 5V offset gear motors mounted in the rear. Pololu TB6612FNG Dual Motor Driver Carrier is used, allowing for convenient steering with PWM. It also uses 2 channels to supply Motors from a second power supply, which is 7.2V Ni-Mh battery pack. Robot is  designed to move along black line on a white surface. Its' decisions are influenced by input from 3 Pololu QTR-3RC reflectance sensor arrays consisting of 3 sensors each. The main components of a sensor are an IR diode and photoresistor. To read the data you need to:
* Set the I/O line to an output and drive it high.
* Wait several microseconds to give the 2.2 nF capacitor node time to reach 5 V.
* Make the I/O line an input (high impedance).
* Measure the time for the capacitor node voltage to decay by waiting for the I/O line to go low.

All modules are firmly mounted on a plexiglass platform, designed and crafted specifically for this purpose.

This Line Follower was built from ground up using CooCox IDE on Windows 8.

## Tools 
Entirety of the project was coded in C with addition of STM32 Repository Libraries in
* #### [CooCox IDE](www.coocox.org/software.html)
Driver used for flashing program into the board is  
* #### [STSW-LINK009](http://www.st.com/content/st_com/en/products/embedded-software/development-tool-software/stsw-link009.html)

## How to compile and run
Code has been reviewed multiple times and is ready to be deployed. To run it on Your STM32 board simply paste it into IDE of Your choice(however please make sure that You have necessary Libraries), compile and flash onto the board.

## Future improvements 
Steering algorithm is very simple and very prone to making wrong decisions.
Both engines operate on very low speed.
Program lacks easily accesible information on current direction of engine rotation.
Major improvement would be an implementation of PID controller.

## Attributions 
We took inspiration from how Line Followers are typically built. Which is two wheels with separate motors in the rear, ballcaster in front and reflectance sensors mounted frontally.

## License
* Distributed under MIT license

## Credits 
Adam Dębczak, Olaf Bergmann, Darek Krajewski  
Computer Science, Faculty of Electical Engineering, Poznan University of Technology.

The project was conducted during the Microprocessor Lab course held by the Institute of Control and Information Engineering, Poznan University of Technology.
Supervisor: Marek Kraft/Michał Fularz/Tomasz Mańkowski/Adam Bondyra
