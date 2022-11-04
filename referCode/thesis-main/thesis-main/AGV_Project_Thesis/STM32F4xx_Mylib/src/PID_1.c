/*
 * PID.c
 *
 *  Created on: Jun 21, 2020
 *      Author: Nhan
 */
#include "PID_1.h"
float pre_ekVeloc = 0,ekVeloc = 0;
float sum_ekVeloc = 0;
float outputVeloc = 0;
float ukVeloc;

float KpVeloc = 3;
float KiVeloc = 4;
float KdVeloc = 0.015;
float Ts_ = 0.01;
float PID_velocity(float setPoint, float current_value)
{
	//if (PIDflag == 0) return 0;
	
	ekVeloc = setPoint - current_value;
	sum_ekVeloc = sum_ekVeloc + Ts_*ekVeloc;
	ukVeloc = KpVeloc*(ekVeloc) + KiVeloc*sum_ekVeloc+ (KdVeloc/Ts_)*(ekVeloc - pre_ekVeloc);
	pre_ekVeloc = ekVeloc;
	
	float outputPWM = ukVeloc*10;
	if (outputPWM > 1000) outputPWM = 1000;
	else if (outputPWM < 0) outputPWM = 0;

	return outputPWM;
 }

 float error_line,position1 = 1000;
float lastError = 0;
float sum_error = 0;
float Kp = 0.5;
float Ki = 0.0005;
float Kd = 0.05;
 	float delta ;

float PID_line(float udk)
{
	
	
	error_line = 1000 - position1;
	sum_error = sum_error + Ts_*error_line;
	delta = Kp*error_line + Ki*sum_error + (Kd/Ts_)*(error_line - lastError);
	lastError = error_line;
	
	if (delta > udk) delta = udk;
	else if (delta < -udk) delta = -udk;
	return delta;
	

}
