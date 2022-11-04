#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX - kết nối với ESP866

#define led   A4
// Motor trai
int enA = 5;
int trai_tien = 2;
int trai_lui = 3;
// Motor phai
int enB = 6;
int phai_tien = 4;
int phai_lui= 7;

// danh cho cam bien do duong
const int cambien_trai1 = A4;        
const int cambien_giua =  A5;      
const int cambien_phai1 = 9;       
//
int i;
int mode= 1;
//

// for wifi 

String wifiString;
unsigned char Counterstringwifi;
int trai1;      
int trungtam;      
int phai1;
///////////////
int tren_trai;
int tren_giua;
int tren_phai;
char c;
void setup()
{
  Serial.begin(9600);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode (phai_tien, OUTPUT);    
  pinMode (phai_lui, OUTPUT);   
  pinMode (trai_tien,    OUTPUT);    
  pinMode (trai_lui,    OUTPUT);    
  pinMode (cambien_trai1, INPUT); 
  pinMode (cambien_giua, INPUT) ;
  pinMode (cambien_phai1, INPUT);
  pinMode (led, OUTPUT); 

  mySerial.begin(115200); // Chọn đúng với tốc độ baud của ESP8266
  // khoi tao esp8266
  mySerial.println("AT+RST");
  delay(5000);
  mySerial.println("AT+CIPMUX=1");
  delay(1500);
  mySerial.println("AT+CIPSERVER=1,8888");// tạo port 8888 cho ESP8266
  delay(2000);
  Serial.println("Setup Done");
  ////////////////////////////
 digitalWrite(led,1);
}

void quay(int quaytrai, int quayphai)
{
  digitalWrite (phai_tien, HIGH);
  digitalWrite (phai_lui, LOW);
  analogWrite (enA, quaytrai);
  digitalWrite (trai_tien, HIGH);
  digitalWrite (trai_lui, LOW);  
  analogWrite (enB, quayphai);
}
void tien(int pwmA, int pwmB)
{
  digitalWrite (phai_tien, HIGH);
  digitalWrite (phai_lui, LOW);
  analogWrite (enA, pwmA);
  digitalWrite (trai_tien, HIGH);
  digitalWrite (trai_lui, LOW);  
  analogWrite (enB, pwmB);
}
void lui(int pwmA, int pwmB)
{
  digitalWrite (phai_tien, LOW);
  digitalWrite (phai_lui, HIGH);
  analogWrite (enA, pwmA);
  digitalWrite (trai_tien, LOW);
  digitalWrite (trai_lui, HIGH);  
  analogWrite (enB, pwmB);
}
void dung()
{
  digitalWrite (phai_tien, LOW);
  digitalWrite (phai_lui, LOW);  
  digitalWrite (trai_tien, LOW);
  digitalWrite (trai_lui, LOW); 
  
}

void loop ()
{
  Bluecontrol();
  if (mode == 2) 
  {
    trai1 = digitalRead (cambien_trai1);
  trungtam = digitalRead (cambien_giua);
  phai1 = digitalRead (cambien_phai1);  
  ////////////////////////////////
    if (trungtam == HIGH) //
    {
      if (trai1 == LOW & phai1 == HIGH) // 
      {
        quay(180, 0);
      }
      else if (phai1 == LOW & trai1 == HIGH) // 
      {
        quay(0, 180);
      }
      else    // 
      {
        tien(255, 255);
      }
    }

    if (trungtam == LOW)  // 
    {
      if (trai1 == LOW & phai1 == HIGH) // 
      {
        quay(180, 0);
      }
      else if (phai1 == LOW & trai1 == HIGH) // 
      {
        quay(0, 180);
      }
      else // are white, stop
      {
        lui(180, 180);
      }
    }    
  }

}
void Bluecontrol()
{
  if(Counterstringwifi > 0)
  {
    Counterstringwifi--;
  }
  if(Counterstringwifi == 1)
  {
    //mySerial.println(wifiString);
    for(int dem = 0; dem< wifiString.length(); dem++)
    {
      if(wifiString[dem] == ':')
      {
        c = (char)wifiString[dem+1];
      }
    }
    wifiString="";
  }
  if( Serial.available())
  {
    c = (char)Serial.read();
  }
  if( mySerial.available())
  {
    wifiString += (char)mySerial.read();
    Counterstringwifi = 100;
  }
  
  if( c == 'U')
  {
    tien(255,255);
    for(int i=0; i< 1000; i++)
    {
      delay(1);
      if(mySerial.available())
      break;
    }
    c= 0;
    dung();
  }
  
  if( c == 'D')
  {
    lui(255,255);
    for(int i=0; i< 1000; i++)
    {
      delay(1);
      if(mySerial.available())
      break;
    }
    c= 0;
    dung();
  }
  
  if( c == 'L')
  {
    quay(255,0);
    for(int i=0; i< 500; i++)
    {
      delay(1);
      if(mySerial.available())
      break;
    }
    c= 0;
    dung();
  }
  
  if( c == 'R')
  {
    quay(0,255);
    for(int i=0; i< 500; i++)
    {
      delay(1);
      if(mySerial.available())
      break;
    }
    c= 0;
   dung();
  }
  
  if( c == 'C')
  {
    dung();
    mode = 1;
  }
  
  if( c == 'e')
  {
    mode = 2;
  }
}
