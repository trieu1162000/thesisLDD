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
        rc522_fd = open("/dev/rfid_rc522_dev", O_RDWR);
        if(rc522_fd == -1)
        {
                perror("open rc522");        /* error check */
                exit(1);
        }
        lcd_fd = open("/dev/lcd0", O_WR);
        if(rc522_fd == -1)
        {
                perror("open rc522");        /* error check */
                exit(1);
        }
        hcsr04_fd = open("/dev/hcsr04", O_RD);
        if(rc522_fd == -1)
        {
                perror("open rc522");        /* error check */
                exit(1);
        }
        tcrt5000_fd = open("/dev/tcrt5000", O_RD);
        if(rc522_fd == -1)
        {
                perror("open rc522");        /* error check */
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
        status = pthread_setschedparam(main_id, policy, &param);
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


        }

        return 0; 
} 
