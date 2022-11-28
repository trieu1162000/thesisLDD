#ifndef _USER_SPACE_APP_MAIN_H_
#define _USER_SPACE_APP_MAIN_H_

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>

enum selected_mode{AUTO, MANUAL};
enum direction{STOP, FORWARD, BACKWARD, TURN_LEFT, TURN_RIGHT};
enum selected_mode mode;
enum direction control_motor;
pthread_t send_thread_id, receive_thread_id, main_thread_id;
pthread_attr_t attr_send_thread, attr_receive_thread;         /* thread attribute structures */
int rfid_rc522_fd, i2c_lcd_fd, hcsr04_fd, uart_cc2530_fd, tcrt5000_fd, pwm_motor_fd;
int policy;
int priority_min, priority_max;      /* for range of priority levels */
struct sched_param param;            /* scheduling structure for thread attributes */
struct robot_info{
    enum selected_mode mode;
    uint velocity;
    uint battery;
    struct manual{
        enum direction control_motor;
    }
    struct auto{
        uint current_node;
        uint next_node;
        uint obstacle_distance;
        enum direction orient;
    }


}

#endif 