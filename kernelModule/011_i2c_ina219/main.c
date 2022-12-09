#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include "ina219_ioctl.h"

int file_desc;

int ioctl_bus_voltage(int file_desc, int16_t *msg)
{
 int ret_val;

 ret_val = ioctl(file_desc, IOCTL_BUS_VOLTAGE,msg);
 return ret_val;
}

int ioctl_shunt_voltage(int file_desc, int16_t *msg)
{
 int ret_val;

 ret_val = ioctl(file_desc, IOCTL_SHUNT_VOLTAGE,msg);
 return 0;
}

int ioctl_measured_current(int file_desc, int16_t *msg)
{
 int ret_val;

 ret_val = ioctl(file_desc, IOCTL_MEASURED_CURRENT,msg);
 return 0;
}

int ioctl_power(int file_desc, int16_t *msg)
{
 int ret_val;

 ret_val = ioctl(file_desc, IOCTL_POWER,msg);
 return 0;
}

int main(void)
{
    int ret_val;
    int16_t recv_msg;
    float bus_voltage,shunt_voltage,measured_current,power;

    file_desc = open(DEVICE_FILE_NAME,0);

    if(file_desc<0)
    {
    printf("Device Open Failed for %s\n",DEVICE_FILE_NAME);
    exit(-1);
    }

    while(1)
    {
        ioctl_bus_voltage(file_desc,&recv_msg);
        bus_voltage = (float)recv_msg/1000;
        printf("Bus Voltage:%f ",bus_voltage);

        ioctl_shunt_voltage(file_desc,&recv_msg);
        shunt_voltage = (float)recv_msg;
        printf("Shunt Voltage:%f ",shunt_voltage);

        ioctl_power(file_desc,&recv_msg);
        power = (float)recv_msg/1024.0;
        printf("Power:%f\n", power);

        ioctl_measured_current(file_desc,&recv_msg);
        measured_current = (float)recv_msg/1000;
        printf("Current:%f\n",measured_current);
        sleep(2);

    }

}