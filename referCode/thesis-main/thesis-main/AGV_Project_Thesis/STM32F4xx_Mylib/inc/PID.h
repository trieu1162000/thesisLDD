
#ifndef PID_PID_H_
#define PID_PID_H_

#include "main.h"

typedef struct
{
	float Kp;
	float Ki;
	float Kd;
	float ek;
	float ek_1;
	float ek_2;
	float uk;
	float uk_1;
	float Ts_;

}PID_Para;

float V_1Ts(float V); // ham lay mau
void PID_Init(void); // ham khoi tao
//void PID_Lift_Init(void);
void PID_GA25_Lifting(float x_ref, float x_measure); // Vd_1 RPM
float PID_Velocity(float x_ref, float x_measure);
void PID_Line( float x_measure,float udk);
void PID_Turn_Left(float x_ref, float x_measure) ;
void PID_Turn_Right(float x_ref, float x_measure) ;

#endif /* PID_PID_H_ */
