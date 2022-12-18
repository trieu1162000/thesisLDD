#ifndef _USER_SPACE_APP_MAIN_H_
#define _USER_SPACE_APP_MAIN_H_

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
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
#include <sys/time.h>  
#include <stdbool.h>

#define BLOCK_SIZE 16
#define MAX_BUF_SIZE 256
#define ENCODER_FILE "/dev/encoder"
#define UART_FILE "/dev/serial0"
#define RFID_FILE "/dev/rfid_rc522"
#define TCRT5000_FILE "/dev/tcrt5000"
#define PWM_LEFT_FILE "/dev/pwm_motor0"
#define PWM_RIGHT_FILE "/dev/pwm_motor1"
#define LCD_FILE "/dev/lcd1"
#define DIRECTION_FILE "/dev/direction_motor"
#define HCSR04_O_FILE "/dev/hcsr04_obstacle"
#define HCSR04_WC_FILE "/dev/hcsr04_w_capacity"
#define INA219_FILE "/dev/ina219"
#define IOCTL_BUS_VOLTAGE _IOWR(156, 0, int16_t *)
#define IOCTL_SPEED_LEFT _IOWR(158, 0, int16_t *)
#define IOCTL_SPEED_RIGHT _IOWR(158, 1, int16_t *)
#define IOCTL_READ_TCRT5000 _IOWR(159, 0, int16_t *)
#define IOCTL_GET_ID 0x8004CE0C

uint8_t tag_id[16][4] = {{0x88, 0x1d, 0xf6, 0x31},
                        {0x88, 0x1d, 0xf5, 0x31},
                        {0x88, 0x1d, 0xf4, 0x31},
                        {0x88, 0x1d, 0xf3, 0x31}, 
                        {0x88, 0x1d, 0xf2, 0x31},
                        {0x88, 0x1d, 0xf1, 0x31},
                        {0x88, 0x1d, 0xf0, 0x31},
                        {0x88, 0x1d, 0xef, 0x31},
                        {0x88, 0x1d, 0xee, 0x31},
                        {0x88, 0x1d, 0xed, 0x31},
                        {0x88, 0x1d, 0x64, 0x32},
                        {0x88, 0x1d, 0x65, 0x32},
                        {0x88, 0x1d, 0x66, 0x32},
                        {0x88, 0x1d, 0x67, 0x32},
                        {0x88, 0x1d, 0x68, 0x32},
                        {0x88, 0x1d, 0x69, 0x32}};

enum selected_mode{CHARGING, AUTO, MANUAL};
enum direction{STOP, FORWARD, BACKWARD, TURN_LEFT, TURN_RIGHT};
enum orient{N, S, W, E};
pthread_t send_thread_id, receive_thread_id, main_thread_id;
pthread_attr_t attr_send_thread, attr_receive_thread;         /* thread attribute structures */
int encoder_fd, rfid_rc522_fd, i2c_lcd_fd, hcsr04_obstacle_fd, uart_fd, tcrt5000_fd,\
    motor_right_fd, motor_left_fd, direction_motor_fd, ina219_fd, hcsr04_w_capacity_fd;
int left_motor_speed, right_motor_speed;
int policy;
int priority_min, priority_max;      /* for range of priority levels */
struct sched_param param;            /* scheduling structure for thread attributes */
int control_motor = STOP;
int robot_mode = 0;

// Send
// robot_info: 0xA0
// auto_info: 0xA1

// Receive
// mode: 0xB0
// path_run: 0xC0
// header - functional code - node0, dir0 - noden, dirn - nodeG
// path_run_back: 0xC1
// start_task_node: 0xC2

// direction: 0xD0
// 

/* For send purpose */
typedef struct{
	uint16_t header;
	uint8_t functional_code; // 
    uint8_t current_node;
    uint8_t current_orient;
    uint8_t obstacle_distance;
    // bool task_incomplete; // when robot is in low battery and low water capacity
    uint8_t task_complete;
    uint16_t EOFrame;
}__attribute__((packed)) auto_info;
auto_info send_auto_info;

typedef struct{
	uint16_t header;
	uint8_t functional_code; // 0x01
	uint8_t velocity;
    uint8_t battery;
    uint8_t water_capacity; 
	uint16_t EOFrame;
}__attribute__((packed)) robot_info;
robot_info send_robot_info;


uint8_t path_run[32];
uint8_t path_run_back[32];
uint start_task_node = -1;
int line_measure_value;
int velocity_initial = 0;
int speed_left_value = 0;
int speed_right_value = 0;
int ina219_value = 0;
char hcsr04_w_capacity_buf[MAX_BUF_SIZE];
char hcsr04_obstacle_buf[MAX_BUF_SIZE];
char tcrt5000_buf[MAX_BUF_SIZE];
char direction_buf[MAX_BUF_SIZE];
char left_speed_buf[MAX_BUF_SIZE];
char right_speed_buf[MAX_BUF_SIZE];
float p_part, i_part, d_part;
float k_p = 0.03, k_i =0.00005, k_d =0;
int error, pre_error, pre_pre_error, pre_out;
float out_line;
struct timeval start, end;
float ts;
char rx_data[256];
int rx_data_len;
char receive_data, pre_receive_data;
int flag_receiver = 0;
int flag_run_back = 0;
uint8_t pre_orient = 'N';
uint8_t pre_node = -1;
int path_run_back_count, path_run_count;
int i = 0;
uint8_t way_to_go;
char* user_id;
int rfid_flag;
int low_battery_flag = 0;
int low_water_flag = 0;
void pid_line();
int set_baudrate(int fd, speed_t speed);
int get_baudrate(int baudrate);
bool back_to_start = false;
void turn_left();
void turn_right();
// void reserve_straight();
int get_node();
void stop();
int compare_tag(uint8_t *tag_read, uint8_t *tag_compare);



#endif 