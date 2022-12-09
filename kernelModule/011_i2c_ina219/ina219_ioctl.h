#ifndef CHAR_CONFIG_H
#define CHAR_CONFIG_H

#include <linux/ioctl.h>

#define MAGIC_NUM 156

//IOCTL interface prototypes
#define IOCTL_BUS_VOLTAGE _IOWR(MAGIC_NUM, 0, int16_t *)
#define IOCTL_SHUNT_VOLTAGE _IOWR(MAGIC_NUM, 1, int16_t *)
#define IOCTL_MEASURED_CURRENT _IOWR(MAGIC_NUM, 2, int16_t *)
#define IOCTL_POWER _IOWR(MAGIC_NUM, 3, int16_t *)

//Device file interface
#define DEVICE_FILE_NAME "/dev/ina219"

#endif