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
        int status;

        /* Files descriptor opening */
        rfid_rc522_fd = open("/dev/rfid_rc522", O_RDWR);
        if(rfid_rc522_fd == -1)
        {
                perror("open rfid_rc522 device file\n");        /* error check */
                exit(1);
        }
        i2c_lcd_fd = open("/dev/lcd0", O_WRONLY);
        if(i2c_lcd_fd == -1)
        {
                perror("open i2c_lcd device fil\ne");        /* error check */
                exit(1);
        }
        hcsr04_fd = open("/dev/hcsr04", O_RDONLY);
        if(hcsr04_fd == -1)
        {
                perror("open hcsr04 device file\n");        /* error check */
                exit(1);
        }
        tcrt5000_fd = open("/dev/tcrt5000", O_RDONLY);
        if(tcrt5000_fd == -1)
        {
                perror("open tcrt5000 device file\n");        /* error check */
                exit(1);
        }
        uart_cc2530_fd = open("/dev/uart_cc2530", O_RDWR);
        if(uart_cc2530_fd == -1)
        {
                perror("open uart_cc2530 device file\n");        /* error check */
                exit(1);
        }
        pwm_motor_fd = open("/dev/pwm_motor", O_WRONLY);
        if(pwm_motor_fd == -1)
        {
                perror("open pwm_motor device file\n");        /* error check */
                exit(1);
        }


        /* Find priority limits */
        policy = SCHED_FIFO;
        priority_max = sched_get_priority_max(policy);
        priority_min = sched_get_priority_min(policy);

        /* Set priority and policy of main thread */
        main_thread_id = pthread_self();
        param.sched_priority = priority_max;
        param.sched_priority=priority_max;
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
                        read(tcrt5000_fd, tcrt5000_buf, sizeof(tcrt5000_buf));
                        line_measure_value = atoi(tcrt5000_buf);
                        PID_line(line_measure_value, velocity_initial);
                }


        }

        return 0; 
} 


void PID_Line(float line_measure_value, float velocity)
{

        int error = line_measure_value - 1000;
        int out_line = setkp_line * error + setkd_line * (error - preError1);
        preError1 = error;
        
        if (out_line > velocity) out_line  = velocity;
        else if (out_line  < -velocity) out_line  = -velocity;
        
        int rightMotorSpeed = velocity + 2 + out_line/1.6;       //1.5       
        int leftMotorSpeed = velocity - out_line/1.6;  		
        
        write(pwm_motor_left_fd, leftMotorSpeed);
        write(pwm_motor_right_fd, rightMotorSpeed);

}