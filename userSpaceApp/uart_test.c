#include "main.h"


void *receive_thread_func(void *pArg)
{
        // printf("receive_thread call\n");

        while (1){

                // receive data in rx_data
                rx_data_len = read(uart_fd, rx_data, 255);
                for(int i = 0; i<rx_data_len; i++)
                        printf("%d\n", rx_data[i]);
                printf("recive %d\n", rx_data_len);
                
                // if(rx_data[4] == 'N')
                //         printf("true\n");

                sleep(1);
                // if(rx_data[0] == 15)
                //         printf("True\n");

	        // if (receiver_flag == 0 && rx_data[0] == 0xAA && rx_data[1] == 0xEE)		
                //         receiver_flag = 1;
                // if(receiver_flag  == 1)
                // {
                //         // mode 
                //         if (rx_data[2] == 0xB0)
                //                 robot_mode = rx_data[3];
                        
                //         // path_run
                //         else if (rx_data[2] == 0xC0)
                //         {
                //                 for(int i = 3; i< rx_data_len - 2; i++)
                //                 {
                //                         path_run[i-3] = rx_data[i];
                //                 }
                //                 // turn task_complete to false
                //                 send_robot_info.task_complete_flag = 1;
                //                 run_task_flag = 1;
                //                 path_run_count = rx_data_len - 5;
                //         }

                //         // path_run_back
                //         else if (rx_data[2] == 0xC1)
                //         {
                //                 for(int i = 3; i< rx_data_len - 2; i++)
                //                 {
                //                         path_run_back[i-3] = rx_data[i];
                //                 }
                //                 run_back_flag = 1;
                //                 path_run_back_count = rx_data_len -5;
                //         }

                //         // start_task_node
                //         else if (rx_data[2] == 0xC2)
                //         {
                //                 start_task_node = rx_data[3];
                //         }

                //         // direction
                //         else if (rx_data[2] == 0xD0)
                //         {
                //                 control_motor = rx_data[3];
                //         }

                //         if(rx_data[rx_data_len - 2] == 0x0A && rx_data[rx_data_len - 1] == 0x0D)
                //         {
                //                 receiver_flag = 0;
                //         }

                // }

                // if(receiver_flag == 1)
                // {
                //         if(rx_data[rx_data_len - 2] == 0x0A && rx_data[rx_data_len - 1] == 0x0D)
                //         {
                //                 receiver_flag = 0;
                //         }
                // }


        }


}

void *send_thread_func(void *pArg)
{
        /* Initialize variables */
        send_robot_info.header = 0xAAEE;
        send_robot_info.functional_code = 0xA1;
        send_robot_info.EOFrame = 0xAEAE;
        // send_robot_info.length = sizeof(send_robot_info);

        // send_robot_info.header = 0xAAEE;
        // send_robot_info.functional_code = 0xA1;
        // send_robot_info.length = sizeof(send_robot_info);
        // send_robot_info.EOFrame = 0xAEAE;
        while(1)
        {
                /* always send robot info */
                // robot.velocity= v_measure;
                send_robot_info.velocity = 10; 
                send_robot_info.current_node = pre_node;
                send_robot_info.current_orient = pre_orient;
                send_robot_info.battery = 0x01;
                send_robot_info.water_capacity = 0x01;
                robot_info temp = send_robot_info;
                printf("sent...\n");
                // write(uart_fd, &temp, sizeof(robot_info));
                // char a[] = "abcd";
                // write(uart_fd, &a, sizeof(a));
                
                // fsync(uart_fd);

                // delay 100 ms
                // usleep(100000);
                sleep(3);
        }
}

int main(int argc, char** argv) 
{
        // struct termios options; /* Serial ports setting */

	// uart_fd = open(UART_FILE, O_RDWR | O_NDELAY | O_NOCTTY);
	// if (uart_fd < 0) {
	// 	perror("Error opening serial port");
	// 	return -1;
	// }
        // set_baudrate(uart_fd, get_baudrate(9600));
        uart_fd = open(UART_FILE, O_RDWR);
        if(uart_fd == -1)
        {
                perror("open uart_cc2530 device file.\n");        /* error check */
                exit(1);
        }
        int status;
        /* Find priority limits */
        policy = SCHED_FIFO;
        priority_max = sched_get_priority_max(policy);
        priority_min = sched_get_priority_min(policy);

        printf("Max: %d, min: %d", priority_max, priority_min);

        /* Create send thread */
        param.sched_priority = priority_max--;
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
        param.sched_priority = priority_max--;
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
        //     send_robot_info.velocity ++;
        //     if(send_robot_info.velocity > 100)
        //         send_robot_info.velocity = 1;
            sleep(1);
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

int get_baudrate(int baudrate)
{        
    switch (baudrate) {
        case 4800:
                return B4800;
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