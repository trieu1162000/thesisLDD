#include "main.h"

/**
 * @brief This thread is used to send the data remotely and display in the GUI.
 */
void *send_thread_func(void *pArg)
{

}

/**
 * @brief This thread is used to receive the data remotely from the GUI for implement goal. 
 */
void *receive_thread_func(void *pArg)
{

}

/**
 * @brief Main function is seemed like a thread and is used for implementation 
 ** base on data from Send/Receive thread.
 */
int main(int argc, char** argv) 
{ 
        int status, result;
        printf("Hello World\n");
        /* Files descriptor opening */
        if (argc != 2) {
                printf("Invalid input, syntax: %s <baudrate> \n", argv[0]);
                return 0;
        }

	uart_fd = open(UART_FILE, O_RDWR | O_NDELAY | O_NOCTTY);
	if (uart_fd < 0) {
		perror("Error opening serial port");
		return -1;
	}

        rfid_rc522_fd = open(RFID_FILE, O_RDWR);
        if(rfid_rc522_fd == -1)
        {
                perror("open rfid_rc522 device file.\n");        /* error check */
                exit(1);
        }

        i2c_lcd_fd = open(LCD_FILE, O_WRONLY);
        if(i2c_lcd_fd == -1)
        {
                perror("open i2c_lcd device file.\n");        /* error check */
                exit(1);
        }

        hcsr04_fd = open(HCSR04_FILE, O_RDONLY);
        if(hcsr04_fd == -1)
        {
                perror("open hcsr04 device file.\n");        /* error check */
                exit(1);
        }

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
        result = set_baudrate(uart_fd, get_baudrate(atoi(argv[1])));
        memset(tcrt5000_buf, 0, MAX_BUF_SIZE);
        memset(direction_buf, 0, MAX_BUF_SIZE);
        memset(left_speed_buf, 0, MAX_BUF_SIZE);
        memset(right_speed_buf, 0, MAX_BUF_SIZE);

        /* Find priority limits */
        policy = SCHED_FIFO;
        priority_max = sched_get_priority_max(policy);
        priority_min = sched_get_priority_min(policy);

        /* Set priority and policy of main thread */
        main_thread_id = pthread_self();
        param.sched_priority = priority_max;
        param.sched_priority = priority_max;
        status = pthread_setschedparam(main_thread_id, policy, &param);
        if (status != 0) 
                perror("pthread_setschedparam");        /* error check */

        /* Create send thread */
        param.sched_priority = priority_min;
        pthread_attr_init(&attr_send_thread);
        status = pthread_attr_setschedpolicy(&attr_send_thread, policy);
        if (status != 0) 
                perror("pthread_attr_setschedpolicy");  /* error check */
        status = pthread_attr_setschedparam(&attr_send_thread, &param);
        if (status != 0) 
                perror("pthread_attr_setschedparam");   /* error check */
        status = pthread_create(&send_thread_id, &attr_send_thread, send_thread_func, NULL);
        if (status != 0) 
                perror("pthread_create");               /* error check */
        status = pthread_setschedparam(send_thread_id, policy, &param);
        if (status != 0) 
                perror("pthread_setschedparam");

        /**
        * @brief Create receive thread which has a priority bigger than
        ** send thread and smaller than main thread.
        */
        param.sched_priority = priority_min++;
        pthread_attr_init(&attr_receive_thread);
        status = pthread_attr_setschedpolicy(&attr_receive_thread, policy);
        if (status != 0) 
                perror("pthread_attr_setschedpolicy");  /* error check */
        status = pthread_attr_setschedparam(&attr_receive_thread, &param);
        if (status != 0) 
                perror("pthread_attr_setschedparam");   /* error check */
        status = pthread_create(&receive_thread_id, &attr_receive_thread, receive_thread_func, NULL);
        if (status != 0) 
                perror("pthread_create");               /* error check */
        status = pthread_setschedparam(receive_thread_id, policy, &param);
        if (status != 0) 
                perror("pthread_setschedparam");
          
        sprintf(direction_buf, "%d", FORWARD);
        write(direction_motor_fd, direction_buf, 1);
        write(motor_left_fd, "80", 2);
        write(motor_right_fd, "80", 2);
        sleep(1);
        velocity_initial = 70;
        while(1)
        {
                if(mode == MANUAL)
                {
                        // TO DO: 
                        // print lcd  
                        // receive data remotely from GUI
                        // drive motor base on data 
                        switch(control_motor)
                        {
                                case STOP:
                                        break;
                                case FORWARD:
                                        break;
                                case BACKWARD:
                                        break;
                                case TURN_LEFT:
                                        break;
                                case TURN_RIGHT:
                                        break;
                                default:
                                        break;

                        }
                }

                else if(mode == AUTO)
                {
                        // TO DO:
                        // print lcd
                        // read rfid
                        // read hcsr04
                        // read line sendor and drive motor
                        // print lcd 
                        // send data to the GUI 
                }
                else {
                        read(tcrt5000_fd, tcrt5000_buf, 4);
                        line_measure_value = atoi(tcrt5000_buf);
                        pid_line(line_measure_value, velocity_initial);
                        sleep(1);
                        printf("Speed: %d - %d\n", left_motor_speed, right_motor_speed);
                }


        }

        return 0; 
} 


void pid_line(int line_measure_value, int velocity)
{

        int error = line_measure_value - 1000;
        int out_line = setkp_line * error + setkd_line * (error - preError1);
        preError1 = error;
        
        if (out_line > velocity) out_line  = velocity;
        else if (out_line  < -velocity) out_line  = -velocity;
        
        right_motor_speed = velocity + 2 + out_line/1.6;       //1.5       
        left_motor_speed = velocity - out_line/1.6;  		
        
        if(right_motor_speed > 99)
                right_motor_speed = 99;
        else if(right_motor_speed <50)
                right_motor_speed = 50;
        if(left_motor_speed > 99)
                left_motor_speed = 99;
        else if(left_motor_speed <50)
                left_motor_speed = 50;
                
        sprintf(left_speed_buf, "%d", left_motor_speed);
        sprintf(right_speed_buf, "%d", right_motor_speed);

        write(motor_left_fd, left_speed_buf, 2);
        write(motor_right_fd, right_speed_buf, 2);

}

int get_baudrate(int baudrate)
{
        switch (baudrate) {
                case 9600:
                        return B9600;
                case 19200:
                        return B19200;
                case 38400:
                        return B38400;
                case 57600:
                        return B57600;
                case 115200:
                        return B115200;
                case 230400:
                        return B230400;
                case 460800:
                        return B460800;
                case 500000:
                        return B500000;
                case 576000:
                        return B576000;
                case 921600:
                        return B921600;
                case 1000000:
                        return B1000000;
                case 1152000:
                        return B1152000;
                case 1500000:
                        return B1500000;
                case 2000000:
                        return B2000000;
                case 2500000:
                        return B2500000;
                case 3000000:
                        return B3000000;
                case 3500000:
                        return B3500000;
                case 4000000:
                        return B4000000;
                default: 
                        return -1;
        }
}

int set_baudrate(int fd, speed_t speed)
{
        struct termios tty;
        int rc1, rc2;

        if (tcgetattr(fd, &tty) < 0) {
                printf("Error from tcgetattr: %s\n", strerror(errno));
                return -1;
        }
        rc1 = cfsetospeed(&tty, speed);
        rc2 = cfsetispeed(&tty, speed);
        if ((rc1 | rc2) != 0 ) {
                printf("Error from cfsetxspeed: %s\n", strerror(errno));
                return -1;
        }
        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
                printf("Error from tcsetattr: %s\n", strerror(errno));
                return -1;
        }
        tcflush(fd, TCIOFLUSH);  /* discard buffers */

        return 0;
}