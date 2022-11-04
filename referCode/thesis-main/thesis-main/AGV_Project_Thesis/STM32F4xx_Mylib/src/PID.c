
#include "PID.h"

//PID_Para motor1;
float uk1 ;
float sum_err;
PID_Para PID_para_vel,PID_para_line, PID_para_lift, PID_para_Turn_Left, PID_para_Turn_Right;
float Error_value=0, P_part =0,I_part =0,D_part =0;
float out =0;
float pre_Error_value=0,pre_Error=0,Error=0;

extern float setkp_line, setki_line, setkd_line;
extern float setkp_speed, setki_speed, setkd_speed;
extern float setkp_lift, setki_lift, setkd_lift;
extern uint8_t PIDflag;
uint8_t flagPickDropComplete = 0;
extern uint8_t PIDflag_Turn_Right;
extern uint8_t PIDflag_Turn_Left;

void PID_Init() // ham khoi tao
{
	PID_para_vel.Kp = 3;
	PID_para_vel.Ki = 4;
	PID_para_vel.Kd = 0.015;
	PID_para_vel.Ts_ = 0.01;
	PID_para_vel.ek = 0 ;
	PID_para_vel.ek_1 = 0 ;
	PID_para_vel.ek_2 = 0;
	PID_para_vel.uk = 0;
	PID_para_vel.uk_1 = 0;
	
	PID_para_line.Kp = 0.002;
	PID_para_line.Ki = 0.2;
	PID_para_line.Kd = 0.1;
	PID_para_line.Ts_ = 0.01;
	PID_para_line.ek = 0 ;
	PID_para_line.ek_1 = 0 ;
	PID_para_line.ek_2 = 0;
	PID_para_line.uk = 0;
	PID_para_line.uk_1 = 0;
	
	
	PID_para_lift.Kp = 2;
	PID_para_lift.Ki = 2;
	PID_para_lift.Kd = 0.015;
	PID_para_lift.Ts_ = 0.01;
	PID_para_lift.ek = 0 ;
	PID_para_lift.ek_1 = 0 ;
	PID_para_lift.ek_2 = 0;
	PID_para_lift.uk = 0;
	PID_para_lift.uk_1 = 0;
	
	
	PID_para_Turn_Left.Kp = 3;
	PID_para_Turn_Left.Ki = 4;
	PID_para_Turn_Left.Kd = 0.015;
	PID_para_Turn_Left.Ts_ = 0.01;
	PID_para_Turn_Left.ek = 0 ;
	PID_para_Turn_Left.ek_1 = 0 ;
	PID_para_Turn_Left.ek_2 = 0;
	PID_para_Turn_Left.uk = 0;
	PID_para_Turn_Left.uk_1 = 0;
	
	
	
	PID_para_Turn_Right.Kp = 3;
	PID_para_Turn_Right.Ki = 4;
	PID_para_Turn_Right.Kd = 0.015;
	PID_para_Turn_Right.Ts_ = 0.01;
	PID_para_Turn_Right.ek = 0 ;
	PID_para_Turn_Right.ek_1 = 0 ;
	PID_para_Turn_Right.ek_2 = 0;
	PID_para_Turn_Right.uk = 0;
	PID_para_Turn_Right.uk_1 = 0;
	
}

void PID_GA25_Lifting(float x_ref, float x_measure) // v_sv RPM
{
	if (x_measure >= (x_ref-(float)0.4) && x_measure <= (x_ref+ (float)0.4)) 
	{
	     PID_para_lift.uk =0;
		   PID_para_lift.uk_1 = 0;
		   flagPickDropComplete = 1;
  }
	else
	{
		 flagPickDropComplete = 0;
	   PID_para_lift.ek = x_ref - x_measure;
	   PID_para_lift.uk = PID_para_lift.uk_1 + setkp_lift*(PID_para_lift.ek - PID_para_lift.ek_1)
													+ setki_lift*PID_para_lift.Ts_*(PID_para_lift.ek + PID_para_lift.ek_1)*0.5
													+ setkd_lift*(PID_para_lift.ek - 2 * PID_para_lift.ek_1 + PID_para_lift.ek_2)/PID_para_lift.Ts_;

	   PID_para_lift.uk_1 = PID_para_lift.uk;
	   PID_para_lift.ek_2 = PID_para_lift.ek_1;
	   PID_para_lift.ek_1 = PID_para_lift.ek;
	}
	//uk1 = motor1.uk;
	if (PID_para_lift.uk >= 100) PID_para_lift.uk =100;
	else if (PID_para_lift.uk <= -100) PID_para_lift.uk = -100;
		
	Run_Motor(LIFT_MOTOR,PID_para_lift.uk/2);
 	 
}

float PID_Velocity(float x_ref, float x_measure) // v_sv RPM
{
	if (PIDflag == 0) 
	{
		PID_para_vel.uk = 0;
		PID_para_vel.uk_1 = 0;
		return 0;
	}	

	
	PID_para_vel.ek = x_ref - x_measure;
	PID_para_vel.uk = PID_para_vel.uk_1 + setkp_speed*(PID_para_vel.ek-PID_para_vel.ek_1) 
	                   + setki_speed*PID_para_vel.Ts_*(PID_para_vel.ek + PID_para_vel.ek_1)*0.5 
	                   + setkd_speed*(PID_para_vel.ek-2*PID_para_vel.ek_1+PID_para_vel.ek_2)/PID_para_vel.Ts_;

	PID_para_vel.uk_1 = PID_para_vel.uk;
	PID_para_vel.ek_2 = PID_para_vel.ek_1;
	PID_para_vel.ek_1 = PID_para_vel.ek;
	if (PID_para_vel.uk >= 100) PID_para_vel.uk =100;
	else if (PID_para_vel.uk <= -100) PID_para_vel.uk = -100;
	return  PID_para_vel.uk;
}

float error1 = 0;
float preError1 = 0;
float sum_error = 0;

float Kp = 0.5;
float Ki = 0.0005;
float Kd = 0.05;


float P = 0,I=0,D=0,previous_error=0;

void PID_Line(float x_measure,float udk)
{
	if(PIDflag == 0)
	{	 
	   preError1 = 0;
	}
	else if (PIDflag == 1)
	{
	   int error = x_measure - 1000;
     int out_line = setkp_line * error + setkd_line * (error - preError1);
     preError1 = error;
	
   	 if (out_line > udk) out_line  = udk;
  	 else if (out_line  < -udk) out_line  = -udk;
	   
     int rightMotorSpeed = udk + 2 + out_line/1.6;       //1.5       
	   int leftMotorSpeed = udk - out_line/1.6;  		
	
	   Run_Motor(LEFT_MOTOR,leftMotorSpeed);
	   Run_Motor(RIGHT_MOTOR,rightMotorSpeed);
	}
}



void PID_Turn_Right(float x_ref, float x_measure) // v_sv RPM
{
	if (PIDflag_Turn_Right == 0) 
	{
		PID_para_Turn_Right.uk = 0;
		PID_para_Turn_Right.uk_1 = 0;
		
	}	
  else
	{
	   PID_para_Turn_Right.ek = x_ref - x_measure;
     PID_para_Turn_Right.uk = PID_para_Turn_Right.uk_1 + 2*(PID_para_Turn_Right.ek-PID_para_Turn_Right.ek_1) 
	                   + 2*PID_para_Turn_Right.Ts_*(PID_para_Turn_Right.ek + PID_para_Turn_Right.ek_1)*0.5 
	                   + 0.015*(PID_para_Turn_Right.ek-2*PID_para_Turn_Right.ek_1+PID_para_Turn_Right.ek_2)/PID_para_Turn_Right.Ts_;

	   PID_para_Turn_Right.uk_1 = PID_para_Turn_Right.uk;
 	   PID_para_Turn_Right.ek_2 = PID_para_Turn_Right.ek_1;
	   PID_para_Turn_Right.ek_1 = PID_para_Turn_Right.ek;
	
	
	   if (PID_para_Turn_Right.uk >= 100) PID_para_Turn_Right.uk =100;
	   else if (PID_para_Turn_Right.uk <= -100) PID_para_Turn_Right.uk = -100;
     Run_Motor(RIGHT_MOTOR, 0);
	   Run_Motor(LEFT_MOTOR, PID_para_Turn_Right.uk);


  }
}



void PID_Turn_Left(float x_ref, float x_measure) // v_sv RPM
{
	if (PIDflag_Turn_Left == 0) 
	{
		PID_para_Turn_Left.uk = 0;
		PID_para_Turn_Left.uk_1 = 0;
		
	}	
  else
	{
	   PID_para_Turn_Left.ek = x_ref - x_measure;
     PID_para_Turn_Left.uk = PID_para_Turn_Left.uk_1 + 2*(PID_para_Turn_Left.ek-PID_para_Turn_Left.ek_1) 
	                   + 2*PID_para_Turn_Left.Ts_*(PID_para_Turn_Left.ek + PID_para_Turn_Left.ek_1)*0.5 
	                   + 0.015*(PID_para_Turn_Left.ek-2*PID_para_Turn_Left.ek_1+PID_para_Turn_Left.ek_2)/PID_para_Turn_Left.Ts_;

	   PID_para_Turn_Left.uk_1 = PID_para_Turn_Left.uk;
 	   PID_para_Turn_Left.ek_2 = PID_para_Turn_Left.ek_1;
	   PID_para_Turn_Left.ek_1 = PID_para_Turn_Left.ek;
	
	
	   if (PID_para_Turn_Left.uk >= 100) PID_para_Turn_Left.uk =100;
	   else if (PID_para_Turn_Left.uk <= -100) PID_para_Turn_Left.uk = -100;
		 Run_Motor(LEFT_MOTOR, 0);
	   Run_Motor(RIGHT_MOTOR, PID_para_Turn_Left.uk);
  }
}


