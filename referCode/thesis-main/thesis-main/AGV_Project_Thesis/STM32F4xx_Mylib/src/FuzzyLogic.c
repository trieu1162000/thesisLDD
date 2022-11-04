#include "FuzzyLogic.h"
float hlt_hthang(float x,float L,float C1, float C2,float R); 
float max(float x1, float x2);
float min(float x1, float x2);
float giaim_trongtam(float x1, float x2); 
float x1_NB,x1_NS,x1_ZE,x1_PS,x1_PB;
float x2_NB,x2_NS,x2_ZE,x2_PS,x2_PB;
float TSS, MS,Y;
float y_NB = -1,y_NM = -0.7,y_NS = -0.3,y_ZE = 0,y_PS = 0.3, y_PM = 0.7, y_PB =1;
float x = -1;
Fuzzy_Para paras;



float giaim_trongtam(float x1, float x2)
{
	x1_NB = hlt_hthang(x1,-1.72,-1 ,-0.6,-0.2);
	x1_NS = hlt_hthang(x1,-0.6,-0.2, -0.2,0);
	x1_ZE = hlt_hthang(x1,-0.2,0,0,0.2);
	x1_PS = hlt_hthang(x1,0,0.2,0.2,0.6);
	x1_PB = hlt_hthang(x1,0.2,0.6,1,1.2);
	
	x2_NB = hlt_hthang(x2,-1.2,-1,-0.8,-0.4);
	x2_NS = hlt_hthang(x2,-0.8,-0.4, -0.4,0);
	x2_ZE = hlt_hthang(x2,-0.4,0,0,0.4);
	x2_PS = hlt_hthang(x2,0,0.4,0.4,0.8);
	x2_PB = hlt_hthang(x2,0.4,0.8,1,1.2);
// Cac quy tac
paras.beta1= min(x1_NB,x2_NB);
paras.beta2= min(x1_NB,x2_NS);
paras.beta3= min(x1_NB,x2_ZE);
paras.beta4= min(x1_NB,x2_PS);
paras.beta5= min(x1_NB,x2_PB);

paras.beta6= min(x1_NS,x2_NB);
paras.beta7= min(x1_NS,x2_NS);
paras.beta8= min(x1_NS,x2_ZE);
paras.beta9= min(x1_NS,x2_PS);
paras.beta10= min(x1_NS,x2_PB);


paras.beta11= min(x1_ZE,x2_NB);
paras.beta12= min(x1_ZE,x2_NS);
paras.beta13= min(x1_ZE,x2_ZE);
paras.beta14= min(x1_ZE,x2_PS);
paras.beta15= min(x1_ZE,x2_PB);

paras.beta16= min(x1_PS,x2_NB);
paras.beta17= min(x1_PS,x2_NS);
paras.beta18= min(x1_PS,x2_ZE);
paras.beta19= min(x1_PS,x2_PS);
paras.beta20= min(x1_PS,x2_PB);

paras.beta21= min(x1_PB,x2_NB);
paras.beta22= min(x1_PB,x2_NS);
paras.beta23= min(x1_PB,x2_ZE);
paras.beta24= min(x1_PB,x2_PS);
paras.beta25= min(x1_PB,x2_PB);

TSS = 	y_NB*(paras.beta1+paras.beta2+paras.beta6)
			+y_NM*(paras.beta3+paras.beta7+paras.beta11)
			+y_NS*(paras.beta4+paras.beta8+paras.beta12+paras.beta16)
			+y_ZE*(paras.beta5+paras.beta9+paras.beta13+paras.beta17+paras.beta21)
			+y_PS*(paras.beta10+paras.beta14+paras.beta18+paras.beta22)
			+y_PM*(paras.beta15+paras.beta19+paras.beta23)
			+y_PB*(paras.beta20+paras.beta24+paras.beta25);
MS =  paras.beta1+paras.beta2+paras.beta6
		+ paras.beta3+paras.beta7+paras.beta11
		+ paras.beta4+paras.beta8+paras.beta12+paras.beta16
		+ paras.beta5+paras.beta9+paras.beta13+paras.beta17+paras.beta21
		+ paras.beta10+paras.beta14+paras.beta18+paras.beta22
		+ paras.beta15+paras.beta19+paras.beta23
		+ paras.beta20+paras.beta24+paras.beta25;
		Y= TSS/MS;


return Y;
}

float hlt_hthang(float x,float L,float C1, float C2,float R)
{
	float temp;
	if(x < L)
		temp = 0;
    else if (x <= C1)
        temp = (x-L)/(C1-L);
    else if (x <= C2)
        temp = 1;
    else if (x <= R)
        temp = (R-x)/(R-C2);
    else
        temp = 0;
		return temp;
}

float max(float x1, float x2)
{
	float temp;
	if (x1 > x2)
	temp = x1;
	else 
		temp = x2;
 return temp;
}

float min(float x1, float x2)
{
	

	float temp;
	if (x1 > x2)
	temp = x2;
	else 
		temp = x1;
 return temp;
}


float saturation(float x)
{
	float temp;
	if (x<-1)
		temp = -1;
	else if (x>1)
		temp = 1;
	else
		temp =x;
return temp;
}
float sat_100(float x)
	{
	float temp;
	if (x<0)
		temp = 0;
	else if (x>100)
		temp = 100;
	else
		temp = x;
return temp;
	
	}

