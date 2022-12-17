#include "main.h"

/**
 * @brief This thread is used to send the data remotely and display in the GUI.
 */
void *send_thread_func(void *pArg)
{
        // while(1)
        // {
        //         /* always send robot info */
        //         send_robot_info.header = 0xFFAA;
        //         send_robot_info.functional_code = 0xA0;
        //         // robot.velocity= v_measure;
        //         send_robot_info.EOFrame = 0x0A0D;
        //         write(uart_fd, &send_robot_info, sizeof(send_robot_info));
        //         fsync(uart_fd);
        //         usleep(100000);

        //         /* Check and send auto info */
        //         if(robot_mode == AUTO){
        //                 send_auto_info.header = 0xFFAA;
        //                 send_auto_info.functional_code = 0xA1;
        //                 // robot.velocity= v_measure;
        //                 send_auto_info.current_node = pre_node;
        //                 send_auto_info.current_orient = pre_orient;
        //                 send_auto_info.EOFrame = 0x0A0D;
        //                 write(uart_fd, &send_auto_info, sizeof(send_auto_info));
        //                 fsync(uart_fd);
        //         }

        //         /* Check and send manual info */
        //         if(robot_mode == AUTO){
        //                 // nothing to do
        //         }

        // }

}

/**
 * @brief This thread is used to receive the data remotely from the GUI for implement goal. 
 */
void *receive_thread_func(void *pArg)
{
        // printf("receive_thread call\n");

        // while (1){

        //         // receive data in rx_data
        //         rx_data_len = read(uart_fd, rx_data, strlen(rx_data));

	//         if ( flag_receiver == 0 && rx_data[0] == 0xFF && rx_data[1] == 0xAA)		
        //                 flag_receiver = 1;
        //         if(flag_receiver  == 1)
        //         {
        //                 // mode 
        //                 if (rx_data[2] == 0xB0)
        //                         robot_mode = rx_data[3];
                        
        //                 // path_run
        //                 if (rx_data[2] == 0xC0)
        //                 {
        //                         for(int i = 3; i< rx_data_len - 2; i++)
        //                         {
        //                                 path_run[i-3] = rx_data[i];
        //                         }
        //                         // turn task_complete to false
        //                         send_auto_info.task_complete = 1;
        //                         path_run_count = rx_data_len - 5;
        //                 }

        //                 // path_run_back
        //                 else if (rx_data[2] == 0xC1)
        //                 {
        //                         for(int i = 3; i< rx_data_len - 2; i++)
        //                         {
        //                                 path_run_back[i-3] = rx_data[i];
        //                         }
        //                         flag_run_back = 1;
        //                         path_run_back_count = rx_data_len -5;
        //                 }

        //                 // start_task_node
        //                 else if (rx_data[2] == 0xC2)
        //                 {
        //                         start_task_node = rx_data[3];
        //                 }

        //                 // direction
        //                 else if (rx_data[2] == 0xD0)
        //                 {
        //                         control_motor = rx_data[3];
        //                 }



        //         }

        //         if(flag_receiver == 1)
        //         {
        //                 if(rx_data[rx_data_len - 2] == 0x0A && rx_data[rx_data_len - 1] == 0x0D)
        //                 {
        //                         flag_receiver = 0;
        //                 }
        //         }


        // }


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

        // hcsr04_obstacle_fd = open(HCSR04_O_FILE, O_RDONLY);
        // if(hcsr04_obstacle_fd == -1)
        // {
        //         perror("open hcsr04_obstacle device file.\n");        /* error check */
        //         exit(1);
        // }

        // hcsr04_w_capacity_fd = open(HCSR04_WC_FILE, O_RDONLY);
        // if(hcsr04_w_capacity_fd == -1)
        // {
        //         perror("open hcsr04_w_capacity device file.\n");        /* error check */
        //         exit(1);
        // }

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
        ina219_fd = open(INA219_FILE, 0);
        if(ina219_fd == -1)
        {
                perror("open ina219 device file.\n");        /* error check */
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
          
        // sprintf(direction_buf, "%d", FORWARD);
        // write(direction_motor_fd, direction_buf, 1);
        // write(motor_left_fd, "80", 2);
        // write(motor_right_fd, "80", 2);
        // sleep(1);
        // velocity_initial = 70;
        // ts = 200;
        // pre_out = 0;
        send_auto_info.task_complete = 0; // 0: complete, 1: incomplete
        memset(path_run, -1, 32);
        memset(path_run_back, -1, 32);
        robot_mode = 3;
        while(1)
        {
                // // 1. Check pin first, display and warning if needed
                // ioctl(ina219_fd, IOCTL_BUS_VOLTAGE, &ina219_value);
                // // convert Volt to % battery 
                // send_robot_info.battery = 25*ina219_value - 300;
                // if(send_robot_info.battery < 15) // 15%
                // { 
                //         low_battery_flag = 1;
                //         // print lcd
                // }
                // else 
                //         low_battery_flag = 0;
                // // 2. Check water capacity
                // read(hcsr04_w_capacity_fd, hcsr04_w_capacity_buf, 2);
                // send_robot_info.water_capacity = atoi(hcsr04_w_capacity_buf);
                // // convert distance to % capacity 

                // if(send_robot_info.water_capacity < 20)
                // {
                //         low_water_flag = 1;
                //         // print lcd

                // } 
                // else 
                //         low_water_flag = 0;

                // if(flag_run_back == 0)
                // {
                //         if(low_battery_flag == 1 || low_water_flag == 1)
                //         {
                //                 if(robot_mode == AUTO)
                //                 {
                //                         if(flag_run_back == 0)
                //                         {
                //                                 stop();
                //                         }
                //                         while(flag_run_back == 0){
                //                                 // Wait signal to run back
                //                                 sleep(1);
                //                         }

                //                         // Check received path_run_back to back to start node(15) and control
                //                         // path run back will from last current node to start node(15)
                //                         // read rfid to check whether the robot is in last current node or not
                //                         get_node();
                //                         // if not, reserve direction to back to last current node
                //                         if(rfid_flag == 0)
                //                         {
                //                                 turn_left();
                //                                 switch(send_auto_info.current_orient)
                //                                 {
                //                                         case N:
                //                                                 send_auto_info.current_orient = S;
                //                                                 break;
                //                                         case S:
                //                                                 send_auto_info.current_orient = N;
                //                                                 break;
                //                                         case E:
                //                                                 send_auto_info.current_orient = W;
                //                                                 break;
                //                                         case W: 
                //                                                 send_auto_info.current_orient = E;
                //                                                 break;
                //                                 }
                //                         }
                //                         do{
                //                                 get_node();
                //                                 pid_line();
                //                                 sleep(1);
                //                         } while(rfid_flag == 0);

                //                 }
                //                 low_battery_flag = 0;
                //                 low_battery_flag = 0;
                //         }
                // }

                // 3. Check mode
                // MANUAL
                if(robot_mode == MANUAL)
                {
                        // TO DO: 
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
                        // stream camera
                }

                // AUTO
                else if(robot_mode == AUTO)
                {
                        // TO DO:
                        //turn off camera

                        // print lcd

                        // read hcsr04 and stop, warning if needed
                        read(hcsr04_obstacle_fd, hcsr04_obstacle_buf, 2);
                        send_auto_info.obstacle_distance = atoi(hcsr04_obstacle_buf);
                        while(send_auto_info.obstacle_distance < 15)
                        {
                                // Stop 
                                sprintf(direction_buf, "%d", STOP);
                                write(direction_motor_fd, direction_buf, 1);

                                // print lcd

                                sleep(1);

                                // take value again
                                read(hcsr04_obstacle_fd, hcsr04_obstacle_buf, 2);
                                send_auto_info.obstacle_distance = atoi(hcsr04_obstacle_buf);
                        }

                        // check whether run_back or not
                        if(flag_run_back == 1)
                        {

	                        pre_node = get_node();
                                if(rfid_flag == 1) 
                                {
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
                                                        i = 0;    
                                                        flag_run_back = 0;                                     
                                                        break;
                                                }
                                        }
                                }
                        }

                        if(send_auto_info.task_complete == 1)
                        {

	                        pre_node = get_node();
                                if(rfid_flag == 1)
                                {
                                        // turn on spraying
                                        if (pre_node == start_task_node)
                                        {
                                                // turn on pump
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
                                                        stop();
                                                        i = 0;    
                                                        // turn off spraying

                                                        send_auto_info.task_complete = 0;                                     
                                                        break;
                                                }
                                        }
                                }
                        }

                }

                // CHARGING means STOP/PARKING
                else if (robot_mode == CHARGING){
                        sprintf(direction_buf, "%d", STOP);
                        write(direction_motor_fd, direction_buf, 1);
                }
                
                // TESTING MODE
                else
                {
                        // gettimeofday(&start, NULL);
                        read(tcrt5000_fd, tcrt5000_buf, 4);
                        line_measure_value = atoi(tcrt5000_buf);
                        sprintf(direction_buf, "%d", FORWARD);
                        write(direction_motor_fd, direction_buf, 1);
                        // if(line_measure_value != 1000)
                        // {       
                        // pid_line(line_measure_value, velocity_initial);
                        pid_line();
                                // sleep(1);
                                // write(motor_left_fd, "00", 2);
                                // write(motor_right_fd, "00", 2);
                        // }
                        // else 
                        // {
                        //         // write(motor_left_fd, "00", 2);
                        //         // write(motor_right_fd, "00", 2);
                        //         // printf("Speed: 0 - 0\n");
                        //         // usleep(300000);
                        // }   
                        if(line_measure_value == 0){
                                write(motor_left_fd, "00", 2);
                                write(motor_right_fd, "00", 2); 
                                // printf("Speed: 0 - 0\n");
                                break;
                        }
                        // gettimeofday(&end, NULL);
                        // ts = (end.tv_usec - start.tv_usec);
                        //printf("The elapsed time is %d seconds", (end.tv_usec - start.tv_usec));

                }
                // sleep(1);


        }

        return 0; 
}


void pid_line(/*int line_measure_value, int velocity*/)
{
        // error = line_measure_value -1000;
        // p_part = k_p*(error - pre_error);
        // i_part = 0.5*k_i*ts*(error + pre_error);
        // d_part = k_d/ts*( error - 2*pre_error+ pre_pre_error);
        // out_line = pre_out + p_part + i_part + d_part ;
        // pre_pre_error = pre_error;
        // pre_error = error;
        // pre_out = out_line;
        // left_motor_speed = velocity + 2 + out_line;       //1.5       
        // right_motor_speed = velocity - out_line;  
        // printf("Raw Speed 1: %d - %d\n", left_motor_speed, right_motor_speed);
        // printf("out line = %d, p = %f, i = %f, d = %f\n", out_line, p_part, i_part, d_part);

        // if (out_line > 20) out_line  = 20;
        // else if (out_line  < -20) out_line  = -20;
        
        // left_motor_speed = velocity + 2 + out_line;       //1.5       
        // right_motor_speed = velocity - out_line;  		
        // printf("Raw Speed 2: %d - %d\n", left_motor_speed, right_motor_speed);
        // if(right_motor_speed > 90)
        //         right_motor_speed = 90;
        // else if(right_motor_speed <50)
        //         right_motor_speed = 50;
        // if(left_motor_speed > 90)
        //         left_motor_speed = 90;
        // else if(left_motor_speed <50)
        //         left_motor_speed = 50;
                
        // sprintf(left_speed_buf, "%d", left_motor_speed);
        // sprintf(right_speed_buf, "%d", right_motor_speed);

        // write(motor_left_fd, left_speed_buf, 2);
        // write(motor_right_fd, right_speed_buf, 2);
        
        if(line_measure_value == 1000)
        {
                write(motor_left_fd, "70", 2);
                write(motor_right_fd, "75", 2);
        }

        else if(line_measure_value == 1250) //01111 
        {
                write(motor_left_fd, "90", 2);
                write(motor_right_fd, "0", 2);
        
        }
        else if(line_measure_value == 1125 || line_measure_value == 1500 ) //10111 //00111
        {
                write(motor_left_fd, "90", 2);
                write(motor_right_fd, "65", 2);
        }

        else if(line_measure_value == 875 || line_measure_value == 500) //11101 11100
        {
                write(motor_left_fd, "60", 2);
                write(motor_right_fd, "95", 2);
        }

        else if(line_measure_value == 750) //11110
        {
                write(motor_left_fd, "0", 2);
                write(motor_right_fd, "95", 2);
        }



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

void turn_left()
{
        do
        {
                read(tcrt5000_fd, tcrt5000_buf, 4);
                line_measure_value = atoi(tcrt5000_buf);
                sprintf(direction_buf, "%d", TURN_LEFT);
                write(direction_motor_fd, direction_buf, 1);
                write(motor_left_fd, "0", 2);
                write(motor_right_fd, "80", 2);

        } while(line_measure_value != 1000);
        stop();

}

void turn_right()
{
        do
        {
                read(tcrt5000_fd, tcrt5000_buf, 4);
                line_measure_value = atoi(tcrt5000_buf);
                sprintf(direction_buf, "%d", TURN_RIGHT);
                write(direction_motor_fd, direction_buf, 1);
                write(motor_left_fd, "80", 2);
                write(motor_right_fd, "0", 2);

        } while(line_measure_value != 1000);
        stop();
}

void stop()
{
        sprintf(direction_buf, "%d", STOP);
        write(direction_motor_fd, direction_buf, 1);
}

int get_node(void)
{
        char id[4] = {0};
        ioctl(rfid_rc522_fd, IOCTL_GET_ID, id);
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

int compare_tag(uint8_t *tag_read, uint8_t *tag_compare)
{

        for(int i = 0; i < 4; i++)
        {
                if(tag_read[i] != tag_compare[i])
                        return -1;
        }
        return 0;
}
