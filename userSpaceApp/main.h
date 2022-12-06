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
#include <termios.h>
#include <stdlib.h>

#define MAX_BUF_SIZE 256
#define UART_FILE "/dev/serial0"
#define RFID_FILE "/dev/rfid_rc522"
#define TCRT5000_FILE "/dev/tcrt5000"
#define PWM_LEFT_FILE "/dev/pwm_motor0"
#define PWM_RIGHT_FILE "/dev/pwm_motor1"
#define LCD_FILE "/dev/lcd1"
#define DIRECTION_FILE "/dev/direction_motor"
#define HCSR04_FILE "/dev/hcsr04"

enum selected_mode{AUTO, MANUAL};
enum direction{STOP, FORWARD, BACKWARD, TURN_LEFT, TURN_RIGHT};
enum selected_mode mode = 2;
enum direction control_motor;
pthread_t send_thread_id, receive_thread_id, main_thread_id;
pthread_attr_t attr_send_thread, attr_receive_thread;         /* thread attribute structures */
int rfid_rc522_fd, i2c_lcd_fd, hcsr04_fd, uart_fd, tcrt5000_fd,\
    motor_right_fd, motor_left_fd, direction_motor_fd;
int left_motor_speed, right_motor_speed;
int policy;
int priority_min, priority_max;      /* for range of priority levels */
struct sched_param param;            /* scheduling structure for thread attributes */
struct manual_mode{
    enum direction control_motor;
};
struct auto_mode{
    uint current_node;
    uint next_node;
    uint obstacle_distance;
    enum direction orient;
};
struct robot_info{
    enum selected_mode mode;
    uint velocity;
    uint battery;
    struct auto_mode auto_info;
    struct manual_mode manual_info;
};
int line_measure_value;
int velocity_initial;
char tcrt5000_buf[MAX_BUF_SIZE];
char direction_buf[MAX_BUF_SIZE];
char left_speed_buf[MAX_BUF_SIZE];
char right_speed_buf[MAX_BUF_SIZE];
int preError1 = 0;
float setkp_line =0.5, setki_line =0.0005, setkd_line =0.05;
void pid_line(int, int);
int set_baudrate(int fd, speed_t speed);
int get_baudrate(int baudrate);

#endif 