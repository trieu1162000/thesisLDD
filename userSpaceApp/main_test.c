#include "main.h"

int main(int argc, char** argv) 
{ 
    tcrt5000_fd = open(TCRT5000_FILE, O_RDONLY);
    if(tcrt5000_fd == -1)
    {
            perror("open tcrt5000 device file.\n");        /* error check */
            exit(1);
    }

    motor_left_fd = open(PWM_LEFT_FILE, O_WRONLY);
    if(motor_left_fd == -1)
    {
            perror("open motor_left device file.\n");        /* error check */
            exit(1);
    }

    motor_right_fd = open(PWM_RIGHT_FILE, O_WRONLY);
    if(motor_right_fd == -1)
    {
            perror("open motor_right device file.\n");        /* error check */
            exit(1);
    }

    direction_motor_fd = open(DIRECTION_FILE, O_WRONLY);
    if(direction_motor_fd == -1)
    {
            perror("open direction_motor device file.\n");        /* error check */
            exit(1);
    }

    encoder_fd = open(ENCODER_FILE, 0);
    if(encoder_fd == -1)
    {
            perror("open encoder device file.\n");        /* error check */
            exit(1);
    }


    memset(tcrt5000_buf, 0, MAX_BUF_SIZE);
    memset(direction_buf, 0, MAX_BUF_SIZE);
    memset(left_speed_buf, 0, MAX_BUF_SIZE);
    memset(right_speed_buf, 0, MAX_BUF_SIZE);

    sprintf(direction_buf, "%d", FORWARD);
    write(direction_motor_fd, direction_buf, 1);

    write(motor_left_fd, "00", 2);
    write(motor_right_fd, "00", 2);
    // write(motor_left_fd, "82", 2);
    while(1)
    {
        sleep(2);
        ioctl(encoder_fd, IOCTL_SPEED_LEFT, &speed_left_value);
        printf("Speed left: %d\n", speed_left_value);
        ioctl(encoder_fd, IOCTL_SPEED_RIGHT, &speed_right_value);
        printf("Speed right: %d\n", speed_right_value);
        // ioctl(tcrt5000_fd, IOCTL_READ_TCRT5000, &line_measure_value);
        // printf("Value: %d\n", line_measure_value); 
        // pid_line();
        // write(motor_left_fd, left_speed_buf, 2);
        // write(motor_right_fd, right_speed_buf, 2);
        // printf("Left value: %d, right value: %d\n", left_motor_speed, right_motor_speed);
        // if(line_measure_value == 1000)
        // {
        //         write(motor_left_fd, "60;666", 2);
        //         write(motor_right_fd, "57", 2);
        // }

        // else if(line_measure_value == 1250) //11110 
        // {
        //         write(motor_left_fd, "70", 2);
        //         write(motor_right_fd, "0", 2);
        
        // }
        // else if(line_measure_value == 1125 || line_measure_value == 1500 ) //11101 //11100
        // {
        //         write(motor_left_fd, "70", 2);
        //         write(motor_right_fd, "55", 2);
        // }

        // else if(line_measure_value == 875 || line_measure_value == 500) // 10111 00111
        // {
        //         write(motor_left_fd, "57", 2);
        //         write(motor_right_fd, "72", 2);
        // }

        // else if(line_measure_value == 750) // 01111
        // {
        //         write(motor_left_fd, "0", 2);
        //         write(motor_right_fd, "72", 2);
        // }
    }
    return 0;
}

void pid_line()
{
    error = line_measure_value - 1000;
    out_line = k_p*error + k_d*(error - pre_error);
    pre_error = error;
    left_motor_speed = (int) (30.0 + out_line);
    right_motor_speed = (int) (28.0 - out_line);
    if(left_motor_speed < 3)
        left_motor_speed = 3;
    else if(left_motor_speed > 99)
        left_motor_speed = 99;
    if(left_motor_speed < 1)
        left_motor_speed = 1;
    else if(left_motor_speed > 97)
        left_motor_speed = 97;
    sprintf(left_speed_buf, "%d", left_motor_speed);
    sprintf(right_speed_buf, "%d", right_motor_speed);
}