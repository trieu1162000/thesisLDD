#include "main.h"

void *send_thread_func(void *pArg)
{
        /* Initialize variables */
        send_robot_info.header = 0xAAEE;
        send_robot_info.functional_code = 0xA0;
        send_robot_info.EOFrame = 0xAEAE;
        // send_robot_info.length = sizeof(send_robot_info);

        send_data_info.header = 0xAAEE;
        send_data_info.EOFrame = 0xAEAE;
        while(1)
        {
                // printf("thread send called \n");
                /* always send robot info */
                // robot.velocity= v_measure;
                send_robot_info.current_node = pre_node;
                send_robot_info.current_orient = pre_orient;
                
                write(uart_fd, &send_robot_info, sizeof(send_robot_info));
                fsync(uart_fd);

                // delay 100 ms
                sleep(1);

                // Send ACK received path run task 
                if(send_ACK_task_flag == 1)
                {
                        send_data_info.functional_code = 0xC3;
                        send_data_info.data = 'A';
                        send_ACK_task_flag = 0;
                        write(uart_fd, &send_data_info, sizeof(send_data_info));
                }

                // Send ACK received path run back
                if(send_ACK_back_flag == 1)
                {
                        send_data_info.functional_code = 0xC4;
                        send_data_info.data = 'A';
                        send_ACK_back_flag = 0;
                        write(uart_fd, &send_data_info, sizeof(send_data_info));

                }

                // Send confirm data task complete
                if(task_complete_flag == 1)
                {
                        send_data_info.functional_code = 0xE0;
                        send_data_info.data = 'C';
                        task_complete_flag = 0;
                        write(uart_fd, &send_data_info, sizeof(send_data_info));
                }

                // Send confirm data back complete
                if(back_complete_flag == 1)
                {
                        send_data_info.functional_code = 0xE1;
                        send_data_info.data = 'C';
                        task_complete_flag = 0;
                        write(uart_fd, &send_data_info, sizeof(send_data_info));

                }


        }
}

/**
 * @brief This thread is used to receive the data remotely from the GUI for implement goal. 
 */
void *receive_thread_func(void *pArg)
{
        // printf("receive_thread call\n");

        while (1){
                // printf("thread receive called \n");

                // receive data in rx_data
                rx_data_len = read(uart_fd, rx_data, 255);
                // for(int i = 0; i<rx_data_len; i++)
                //         printf("%d\n", rx_data[i]);
                // printf("recive %d\n", rx_data_len);
           

                // while(rx_data_len <= 0)
                // {
                //         sleep(1);
                //         rx_data_len = read(uart_fd, rx_data, strlen(rx_data));
                // }
                for(int i = 0; i<rx_data_len ; i++)
                {
                        if (receiver_flag == 0 && rx_data[i] == 0xAA && rx_data[i+1] == 0xEE)	
                        {
                                index_start_data = i + 2;
                        }	
                        if (rx_data[i] == 0xAE && rx_data[i+1] == 0xAE)
                        {
                                index_stop_data = i-1;
                                receiver_flag = 1;
                                break;
                        }
                }
                // printf("start\n");
                // for(int i = index_start_data; i<index_stop_data +1; i++)
                //         printf("%d\n", rx_data[i]);
                // printf("stop\n");
                if(receiver_flag  == 1)
                {
                        // mode 
                        if (rx_data[index_start_data] == 0xB0)
                                robot_mode = rx_data[3];
                        
                        // path_run
                        else if (rx_data[index_start_data] == 0xC0)
                        {
                                // printf("called here\n");
                                for(int j = index_start_data + 1; j<= index_stop_data; j++)
                                {
                                        if(rx_data[j] == 16)
                                                rx_data[j] = 0;
                                        path_run[j - index_start_data -1] = rx_data[j];
                                }
                                run_task_flag = 1;
                                send_ACK_task_flag = 1;
                                path_run_count = index_start_data - index_stop_data +1;
                                printf("thread receive: Call here\n");
                        }

                        // path_run_back
                        else if (rx_data[index_start_data] == 0xC1)
                        {
                                for(int j = index_start_data + 1; j<= index_stop_data; j++)
                                {
                                        if(rx_data[j] == 16)
                                                rx_data[j] = 0;
                                        path_run_back[j - index_start_data -1] = rx_data[j];
                                }
                                run_back_flag = 1;
                                send_ACK_back_flag = 1;
                                path_run_count = index_start_data - index_stop_data +1;
                        }

                        // start_task_node
                        else if (rx_data[index_start_data] == 0xC2)
                        {
                                start_task_node = rx_data[3];
                        }

                        // direction
                        else if (rx_data[index_start_data] == 0xD0)
                        {
                                control_pump = rx_data[3];
                        }

                        else if (rx_data[index_start_data] == 0xD1)
                        {
                                control_motor = rx_data[3];
                        }
                        receiver_flag = 0;
                        memset(rx_data, 0, 256);

                }
                sleep(1);

                // if(receiver_flag == 1)
                // {
                //         if(rx_data[rx_data_len - 2] == 0x0A && rx_data[rx_data_len - 1] == 0x0D)
                //         {
                //                 receiver_flag = 0;
                //         }
                // }


        }


}

void *auto_thread_func(void *pArg)
{
        while (1)
        {
                printf("thread auto called \n");

                // Check have task or run back or not
                while(run_back_flag == 0 && run_task_flag == 0)
                {
                        sleep(1);
                        printf("call here 1\n");
                }
                // usleep(10000);          // 10ms

                // Check obstacle 
                read(hcsr04_obstacle_fd, hcsr04_obstacle_buf, 2);
                if(atoi(hcsr04_obstacle_buf) < 10)
                {                        
                        obstacle_flag = 1;
                }
                else 
                        obstacle_flag = 0;
                printf("Distance: %s\n", hcsr04_obstacle_buf);
                // printf("call here 1\n");
                while(obstacle_flag == 1)
                {
                        stop();
                        sleep(1);
                        read(hcsr04_obstacle_fd, hcsr04_obstacle_buf, MAX_BUF_SIZE);
                        if(atoi(hcsr04_obstacle_buf) > 10)
                                obstacle_flag = 0;
                        // printf("Distance: %s\n", hcsr04_obstacle_buf);
                        
                }
                printf("call here 1\n");

                // Check low battery and water
                if(low_water_flag || low_battery_flag)
                {
                        if(run_back_flag == 0)
                        {
                                stop();
                        }

                        // Wait for GUI handle and send path to run back
                        while(run_back_flag == 0)
                                sleep(1);
                        
                        if(reserve_last_node_flag == 0) // this function should do 1 time
                        {
                                reserve_last_node_flag = 1; 
                                // Check in node or not
                                get_node();
                                if(rfid_flag == 1)       // in node
                                        rfid_flag = 0;
                                else                    // not in node
                                {
                                        // reserve to last current node
                                        turn_left();
                                        switch(send_robot_info.current_orient)
                                        {
                                                case N:
                                                        send_robot_info.current_orient = S;
                                                        break;
                                                case S:
                                                        send_robot_info.current_orient = N;
                                                        break;
                                                case E:
                                                        send_robot_info.current_orient = W;
                                                        break;
                                                case W: 
                                                        send_robot_info.current_orient = E;
                                                        break;
                                        }
                                }
                                do
                                {
                                        get_node();
                                        pid_line();
                                        sleep(1);
                                } while(rfid_flag == 0);
                        }

                }

                // Check run task or not 
                if(run_task_flag == 1 && run_back_flag == 0)
                {
                        pre_node = get_node();
                        // turn on spraying
                        if (pre_node == start_task_node)
                        {
                                // turn on pump
                                write(pump_fd, "1", 1 );
                        } 
                        int index_pre_node = 0;
                        for (int i = 2; i < path_run_back_count  - 2; i++)         
                        {
                                
                                if (pre_node == path_run[i])
                                {
                                        index_pre_node = i;
                                        break;
                                }
                        }
                        way_to_go = path_run[index_pre_node + 1];
                        switch (way_to_go)
                        { 
                                case 'N': 
                                {
                                        switch(pre_orient)
                                                {
                                                case 'N': 
                                                        pid_line();  
                                                        pre_orient = 'N';
                                                        break;
                                                case 'S':  
                                                        turn_left();  
                                                        usleep(100000);         // 100ms
                                                        pid_line(); 
                                                        pre_orient = 'N';
                                                        break;
                                                case 'W': 
                                                        turn_right(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'N'; 
                                                        break;
                                                case 'E':  
                                                        turn_left();  
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'N'; 
                                                        break;
                                                }
                                                break;
                                }
                                case 'S': 
                                {
                                        switch(pre_orient)
                                        {
                                                case 'N':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'S'; 
                                                        break;
                                                case 'S':  
                                                        pid_line(); 
                                                        pre_orient = 'S'; 
                                                        break;
                                                case 'W':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'S'; 
                                                        break;
                                                case 'E':  
                                                        turn_right(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'S'; 
                                                        break;
                                        }
                                        break;
                                                        
                                }
                                case 'W':
                                {
                                        switch(pre_orient)
                                        {
                                                case 'N':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'W'; 
                                                        break;
                                                case 'S':  
                                                        turn_right(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'W';
                                                        break;
                                                case 'W':  
                                                        pid_line(); 
                                                        pre_orient = 'W'; 
                                                        break;
                                                case 'E':  
                                                        turn_left();  
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'W'; 
                                                        break;
                                        }
                                        break;
                                }	
                                case 'E':
                                {
                                        switch(pre_orient)
                                        {
                                                case 'N':  
                                                        turn_right(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'E'; 
                                                        break;
                                                case 'S':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'E'; 
                                                        break;
                                                case 'W':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'E';  
                                                        break;
                                                case 'E':  
                                                        pid_line(); 
                                                        pre_orient = 'E'; 
                                                        break;
                                        }
                                        break;
                                }
                                case 'G': 
                                {  
                                        stop();
                                        // turn off spraying
                                        write(pump_fd, "0", 1);
                                        run_task_flag = 0;
                                        task_complete_flag = 1;                                     
                                        break;
                                }
                        }
                }

                // Check run back or not
                if(run_back_flag == 1)
                {

                        pre_node = get_node();
                        int index_pre_node = 0;
                        for (int i = 2; i < path_run_back_count  - 2; i++)         
                        {
                                
                                if (pre_node == path_run_back[i])
                                {
                                        pre_node = i;
                                        break;
                                }
                        }
                        way_to_go = path_run_back[index_pre_node + 1];
                        switch (way_to_go)
                        { 
                                case 'N': 
                                {
                                        switch(pre_orient)
                                                {
                                                case 'N': 
                                                        pid_line();  
                                                        pre_orient = 'N';
                                                        break;
                                                case 'S':  
                                                        turn_left();  
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'N';
                                                        break;
                                                case 'W': 
                                                        turn_right(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'N'; 
                                                        break;
                                                case 'E':  
                                                        turn_left();  
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'N'; 
                                                        break;
                                                }
                                                break;
                                }
                                case 'S': 
                                {
                                        switch(pre_orient)
                                        {
                                                case 'N':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'S'; 
                                                        break;
                                                case 'S':  
                                                        pid_line(); 
                                                        pre_orient = 'S'; 
                                                        break;
                                                case 'W':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'S'; 
                                                        break;
                                                case 'E':  
                                                        turn_right(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'S'; 
                                                        break;
                                        }
                                        break;
                                                        
                                }
                                case 'W':
                                {
                                        switch(pre_orient)
                                        {
                                                case 'N':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'W'; 
                                                        break;
                                                case 'S':  
                                                        turn_right(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'W';
                                                        break;
                                                case 'W':  
                                                        pid_line(); 
                                                        pre_orient = 'W'; 
                                                        break;
                                                case 'E':  
                                                        turn_left();  
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'W'; 
                                                        break;
                                        }
                                        break;
                                }	
                                case 'E':
                                {
                                        switch(pre_orient)
                                        {
                                                case 'N':  
                                                        turn_right(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'E'; 
                                                        break;
                                                case 'S':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'E'; 
                                                        break;
                                                case 'W':  
                                                        turn_left(); 
                                                        usleep(100000);
                                                        pid_line(); 
                                                        pre_orient = 'E';  
                                                        break;
                                                case 'E':  
                                                        pid_line(); 
                                                        pre_orient = 'E'; 
                                                        break;
                                        }
                                        break;
                                }
                                case 'G': 
                                {  
                                        turn_left();
                                        stop();
                                        run_back_flag = 0;   
                                        reserve_last_node_flag = 0;
                                        back_complete_flag = 1;                                  
                                        break;
                                }
                        }
                }

        }
}

void *manual_thread_func(void *pArg)
{
        while (1){
                printf("thread manual called \n");
                usleep(10000);          // 10ms
                // TO DO: 
                // Stream camera
                
                // camera_on_flag = 1;

                // print lcd
                // control_motor 

                // drive motor base on data 
                switch(control_motor)
                {
                        case STOP:
                                stop();
                                break;
                        case FORWARD:
                                sprintf(direction_buf, "%d", FORWARD);
                                write(direction_motor_fd, direction_buf, 1);
                                write(motor_left_fd, "80", 2);
                                write(motor_right_fd, "80", 2);
                                break;
                        case BACKWARD:
                                sprintf(direction_buf, "%d", BACKWARD);
                                write(direction_motor_fd, direction_buf, 1);
                                write(motor_left_fd, "80", 2);
                                write(motor_right_fd, "80", 2);
                                break;
                        case TURN_LEFT:
                                sprintf(direction_buf, "%d", TURN_LEFT);
                                write(direction_motor_fd, direction_buf, 1);
                                write(motor_left_fd, "80", 2);
                                write(motor_right_fd, "80", 2);
                                break;
                        case TURN_RIGHT:
                                sprintf(direction_buf, "%d", TURN_RIGHT);
                                write(direction_motor_fd, direction_buf, 1);
                                write(motor_left_fd, "80", 2);
                                write(motor_right_fd, "80", 2);
                                break;
                        default: // STOP
                                stop();
                                break;
                }
        }
}


int main(int argc, char** argv) 
{


	uart_fd = open(UART_FILE, O_RDWR);
	if (uart_fd == -1) {
		perror("Error opening serial port");
		return -1;
	}

        rfid_rc522_fd = open(RFID_FILE, O_RDWR);
        if(rfid_rc522_fd == -1)
        {
                perror("open rfid_rc522 device file.\n");        /* error check */
                exit(1);
        }

        // i2c_lcd_fd = open(LCD_FILE, O_WRONLY);
        // if(i2c_lcd_fd == -1)
        // {
        //         perror("open i2c_lcd device file.\n");        /* error check */
        //         exit(1);
        // }

        hcsr04_obstacle_fd = open(HCSR04_O_FILE, O_RDONLY);
        if(hcsr04_obstacle_fd == -1)
        {
                perror("open hcsr04_obstacle device file.\n");        /* error check */
                exit(1);
        }

        hcsr04_w_capacity_fd = open(HCSR04_WC_FILE, O_RDONLY);
        if(hcsr04_w_capacity_fd == -1)
        {
                perror("open hcsr04_w_capacity device file.\n");        /* error check */
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
        // ina219_fd = open(INA219_FILE, 0);
        // if(ina219_fd == -1)
        // {
        //         perror("open ina219 device file.\n");        /* error check */
        //         exit(1);
        // }
        pump_fd = open(PUMP_FILE, O_WRONLY);
        if(pump_fd == -1)
        {
                perror("open pump device file.\n");        /* error check */
                exit(1);
        }
        int status;
        /* Find priority limits */
        policy = SCHED_FIFO;
        priority_max = sched_get_priority_max(policy);
        priority_min = sched_get_priority_min(policy);

        printf("Max: %d, min: %d", priority_max, priority_min);
        /* Set priority and policy of main thread */
        main_thread_id = pthread_self();
        param.sched_priority = priority_max;
        status = pthread_setschedparam(main_thread_id, policy, &param);
        if (status != 0) 
                perror("pthread_setschedparam");  

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

        /* Create auto thread */
        param.sched_priority = priority_max - 1 ;
        pthread_attr_init(&attr_auto_thread);
        status = pthread_attr_setschedpolicy(&attr_auto_thread, policy);
        if (status != 0) 
                perror("pthread_attr_setschedpolicy");  /* error check */
        status = pthread_attr_setschedparam(&attr_auto_thread, &param);
        if (status != 0) 
                perror("pthread_attr_setschedparam");   /* error check */
        status = pthread_create(&auto_thread_id, &attr_auto_thread, auto_thread_func, NULL);
        if (status != 0) 
                perror("pthread_create");               /* error check */
        status = pthread_setschedparam(auto_thread_id, policy, &param);
        if (status != 0) 
                perror("pthread_setschedparam");

        /* Create manual thread */
        param.sched_priority = priority_max - 1;
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

        // Connect camera
        system("python3 /home/pi/pi-camera-stream-flask/main.py");

        while(1){
                printf("thread main called \n");
                // 1. Check pin first, display and warning if needed
                // ioctl(ina219_fd, IOCTL_BUS_VOLTAGE, &ina219_value);
                // // convert Volt to % battery 
                // send_robot_info.battery = 25*ina219_value - 300;
                // if(send_robot_info.battery < 15) // 15%
                // { 
                //         low_battery_flag = 1;
                // }
                // else 
                //         low_battery_flag = 0;

                // 2. Check water capacity
                read(hcsr04_w_capacity_fd, hcsr04_w_capacity_buf, 2);
                send_robot_info.water_capacity = -20*(atoi(hcsr04_w_capacity_buf)) + 200;
                // convert distance to % capacity 

                if(send_robot_info.water_capacity < 20)
                {
                        low_water_flag = 1;
                } 
                else 
                        low_water_flag = 0;
                
                // 3. Measure speed
                ioctl(encoder_fd, IOCTL_SPEED_LEFT, &speed_left_value);
                ioctl(encoder_fd, IOCTL_SPEED_RIGHT, &speed_right_value);
                send_robot_info.velocity = (speed_left_value+speed_right_value)/2 - 50;

                // 4. Print LCD 
                // battery
                // water
                // velocity
                // mode


                sleep(1);

        }
}



void pid_line()
{
    error = line_measure_value - 3000;
    out_line = k_p*error + k_d*(error - pre_error);
    pre_error = error;
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
    }
    else if(left_motor_speed > 97)
        left_motor_speed = 97;

    if(right_motor_speed < 0)
    {
        right_motor_speed = 0;
    }
    else if(right_motor_speed > 99)
        right_motor_speed = 99;

    sprintf(left_speed_buf, "%d", left_motor_speed);
    sprintf(right_speed_buf, "%d", right_motor_speed);
}

void turn_left()
{
    do
    {
        ioctl(tcrt5000_fd, IOCTL_READ_TCRT5000, &line_measure_value);
        sprintf(direction_buf, "%d", TURN_LEFT);
        write(direction_motor_fd, direction_buf, 1);
        write(motor_left_fd, "0", 2);
        write(motor_right_fd, "80", 2);

    } while(line_measure_value != 3000);
    stop();
}
void turn_right()
{
    do
    {
        ioctl(tcrt5000_fd, IOCTL_READ_TCRT5000, &line_measure_value);
        sprintf(direction_buf, "%d", TURN_RIGHT);
        write(direction_motor_fd, direction_buf, 1);
        write(motor_left_fd, "80", 2);
        write(motor_right_fd, "0", 2);

    } while(line_measure_value != 3000);
    stop();
}

int get_node(void)
{
        uint8_t id[4] = {0};
        ioctl(rfid_rc522_fd, IOCTL_GET_ID, id);
        // delay 20ms
        usleep(20000);
        for (int i = 0; i < 16; i++)
        {
                uint8_t compare_id[4];
                for (int j = 0; j < 4; j++) 
                        compare_id[j] = tag_id[i][j];
                if (compare_tag(id, compare_id) == 0) 
                {
                        rfid_flag = 1;
                        return i;
                }  

                else 
                {
                        rfid_flag = 0;
                        return pre_node;
                }
        }

}
void stop()
{
        sprintf(direction_buf, "%d", STOP);
        write(direction_motor_fd, direction_buf, 1);
}

int compare_tag(uint8_t *tag_read, uint8_t *tag_compare)
{
        for(int i = 0; i < 4; i++)
        {
                if(tag_read[i] != tag_compare[i])
                        return -1;
        }
        return 0;
}



