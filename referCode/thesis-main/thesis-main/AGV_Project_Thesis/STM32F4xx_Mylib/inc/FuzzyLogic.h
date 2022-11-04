/*
 * FuzzyLogic.h
 *
 *  Created on: Dec 16, 2020
 *      Author: Thien
 */

#ifndef FuzzyLogic_H_
#define FuzzyLogic_H_

#include "main.h"


typedef struct
{
	float beta1,beta2,beta3;
	float beta4,beta5,beta6;
	float beta7,beta8,beta9;
	float beta10,beta11,beta12;
	float beta13,beta14,beta15;
	float beta16,beta17,beta18;
	float beta19,beta20,beta21;
	float beta22,beta23,beta24,beta25;
	float beta_NB,beta_NS,beta_NM,beta_ZE,beta_PS, beta_PM, beta_PB;
}Fuzzy_Para;

float hlt_hthang(float x,float L,float C1, float C2,float R); 
float max(float x1, float x2);
float min(float x1, float x2);
float giaim_trongtam(float x1, float x2); 
float saturation(float x);
float sat_100(float x);
#endif 
