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

static pthread_t send_thread_id, receive_thread_id, main_thread_id;
static pthread_attr_t attr_send_thread, attr_receive_thread;         /* thread attribute structures */
static enum selected_mode{AUTO, MANUAL};
static uint mode;
static int rc522_fd;
static int lcd_fd;
static int policy;
static int priority_min, priority_max;      /* for range of priority levels */
static struct sched_param param;            /* scheduling structure for thread attributes */

#endif 