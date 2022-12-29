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


    memset(direction_buf, 0, MAX_BUF_SIZE);
    memset(left_speed_buf, 0, MAX_BUF_SIZE);
    memset(right_speed_buf, 0, MAX_BUF_SIZE);

    sprintf(direction_buf, "%d", FORWARD);
    write(direction_motor_fd, direction_buf, 1);

    // write(motor_left_fd, "75", 2);
    // write(motor_right_fd, "69", 2);
    // write(motor_left_fd, "72", 2);
    while(1)
    {
        sleep(1);
        // // usleep(10000);
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
        //         write(motor_left_fd, "69;666", 2);
        //         write(motor_right_fd, "57", 2);
        // }

        // else if(line_measure_value == 1250) //11110 
        // {
        //         write(motor_left_fd, "69", 2);
        //         write(motor_right_fd, "0", 2);
        
        // }
        // else if(line_measure_value == 1125 || line_measure_value == 1500 ) //11101 //11100
        // {
        //         write(motor_left_fd, "69", 2);
        //         write(motor_right_fd, "55", 2);
        // }

        // else if(line_measure_value == 881 || line_measure_value == 500) // 10111 00111
        // {
        //         write(motor_left_fd, "57", 2);
        //         write(motor_right_fd, "72", 2);
        // }

        // else if(line_measure_value == 810) // 01111
        // {
        //         write(motor_left_fd, "0", 2);
        //         write(motor_right_fd, "72", 2);
        // }
    }
    return 0;
}

// 77 0.2 0.05
void pid_line()
{
    error = line_measure_value - 3000;
    out_line = k_p*error + k_d*(error - pre_error);
    pre_error = error;
    // if(out_line > 69)
    //     out_line = 69;
    // if(out_line<-69)
    //     out_line = -69;
    left_motor_speed = (int) (86.0 + out_line );
    right_motor_speed = (int) (86.0 - out_line + 2.0);
    if(left_motor_speed > 0 && right_motor_speed > 0)
    {
        sprintf(direction_buf, "%d", FORWARD);
        write(direction_motor_fd, direction_buf, 1);
    }

    if(left_motor_speed < 0)
    {
        left_motor_speed = 0;
        // if(left_motor_speed < -97)
        //     left_motor_speed = 97;
        // else 
        //     left_motor_speed = - left_motor_speed;
        // sprintf(direction_buf, "%d", TURN_RIGHT);
        // write(direction_motor_fd, direction_buf, 1);
    }
    else if(left_motor_speed > 97)
        left_motor_speed = 97;

    if(right_motor_speed < 0)
    {
        right_motor_speed = 0;
        // if(right_motor_speed < -99)
        //     right_motor_speed = 99;
        // else 
        //     right_motor_speed = - right_motor_speed;
        // sprintf(direction_buf, "%d", TURN_LEFT);
        // write(direction_motor_fd, direction_buf, 1);
    }
    else if(right_motor_speed > 99)
        right_motor_speed = 99;

    sprintf(left_speed_buf, "%d", left_motor_speed);
    sprintf(right_speed_buf, "%d", right_motor_speed);
}