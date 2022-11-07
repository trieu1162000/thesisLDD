
#include "qtr_5rc_api.h"

static uint charging = 1;
static uint counter = 0;
uint selected_qtr_5rc_pin = 0;
uint flag_stop_timer = 0;
int sensor_array[NUM_OF_SENSORS];
/**
 * @brief Initializing the GPIO
 */
int qtr_5rc_init_gpio(uint gpio_num)
{
	char label[100];

	//Checking the GPIO is valid or not 
	if(!gpio_is_valid(gpio_num)){
		pr_err("ERROR: GPIO %d is not valid\n", gpio_num);
		return -1;
	}
	//Requesting the GPIO
	sprintf(label,"gpio-%d", gpio_num);
	if(gpio_request(gpio_num, label) < 0){
		pr_err("ERROR: GPIO %d request\n", gpio_num);
		return -1;
	}

	return 0;
}

/**
 * @brief set pin of gpios to the input mode
 */
void qtr_5rc_gpio_input_mode(uint gpio_num)
{
	if(gpio_direction_input(gpio_num)) {
		pr_err("ERROR:Can not set GPIO %d to input!\n", gpio_num);
	}
}

/**
 * @brief set pin of gpios to the output mode and init high value
 */
void qtr_5rc_gpio_output_high(uint gpio_num)
{
	if(gpio_direction_output(gpio_num, 1)) {
		pr_err("ERRORCan not set GPIO %d to output high!\n", gpio_num);
	}
}

void init_sensor(void)
{
    qtr_5rc_gpio_output_high(GPIO_BASE_NUM + selected_qtr_5rc_pin);
    counter = 0;
    charging = 1;
}

/**
 * @brief handler function for high resolution timer
 */
enum hrtimer_restart hrtimer_handler(struct hrtimer *timer) 
{
    if(counter == 2)
    {
        qtr_5rc_gpio_input_mode(GPIO_BASE_NUM + selected_qtr_5rc_pin);
        charging = 0;
    }
    counter ++;

    if(counter > 2)
    {
        if(charging == 0)
        {
            sensor_selection();
        }
    }
    
    hrtimer_forward_now(timer,ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC));
    return HRTIMER_RESTART;
}

/**
 * @brief 	Managing the input from the current sensor
 ** Converting the value of counter to binary (0 = white, 1 = black) 
 ** for each sensor indicated by selectedSensor
 ** Disables the timer after last value is read
 */
void sensor_selection(void) 
{
	switch (selected_qtr_5rc_pin) 
	{
	case 0:
		if (gpio_get_value(GPIO_BASE_NUM + selected_qtr_5rc_pin) == 0) {
			if (is_black(counter))
                sensor_array[selected_qtr_5rc_pin] = 1;
			else 
                sensor_array[selected_qtr_5rc_pin] = 0;
			selected_qtr_5rc_pin = 1;
			init_sensor();
		}
		break;
	case 1:
		if (gpio_get_value(GPIO_BASE_NUM + selected_qtr_5rc_pin) == 0) {
			if (is_black(counter))
                sensor_array[selected_qtr_5rc_pin] = 1;
			else 
                sensor_array[selected_qtr_5rc_pin] = 0;
			selected_qtr_5rc_pin = 2;
			init_sensor();
		}
		break;
	case 2:
		if (gpio_get_value(GPIO_BASE_NUM + selected_qtr_5rc_pin) == 0) {
			if (is_black(counter))
                sensor_array[selected_qtr_5rc_pin] = 1;
			else 
                sensor_array[selected_qtr_5rc_pin] = 0;
			selected_qtr_5rc_pin = 3;
			init_sensor();
		}

		break;
	case 3:
		if (gpio_get_value(GPIO_BASE_NUM + selected_qtr_5rc_pin) == 0) {
			if (is_black(counter))
                sensor_array[selected_qtr_5rc_pin] = 1;
			else 
                sensor_array[selected_qtr_5rc_pin] = 0;
			selected_qtr_5rc_pin = 4;
			init_sensor();
		}
		break;
	case 4:
		if (gpio_get_value(GPIO_BASE_NUM + selected_qtr_5rc_pin) == 0) {
			if (is_black(counter))
                sensor_array[selected_qtr_5rc_pin] = 1;
			else 
                sensor_array[selected_qtr_5rc_pin] = 0;
			selected_qtr_5rc_pin = 0;
			init_sensor();
            flag_stop_timer = 1;
		}
		break;
	default:
		break;
	}

}


int is_black(uint x) 
{
	return x > 15; 
}
