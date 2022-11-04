#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

#define ssid      "P1705"       // WiFi SSID
#define password  "1234567890"        // WiFi password

#define host_ssid "AGV"
#define host_password "12345678"

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputTorchState = "off";
String Speed = "";
String KP = "";
String KI = "";
String KD = "";

// Assign output variables to GPIO pins
const int outputStart = 5;
const int outputPause = 4;
const int outputReady = 0;
const int outputTouch = 2;
const int outputReset = 14; //D5
const int getIP =12;
const int TRIGGER = 13;
const int ECHO = 15;
const int sonar = 16;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


int BAT= A0;              //Analog channel A0 as used to measure battery voltage
float RatioFactor=0.86 ;  //Resistors Ration Factor
float Tvoltage=0.0;
float Vvalue=0.0,Rvalue=0.0;

ESP8266WebServer server (80);

String getPage(){
  //String page = "<html lang=fr-FR><head><meta http-equiv='refresh' content='10'/>";
  String page = "<html lang=fr-FR><head>";
  page += "<title>AGV PROJECT</title>";
  page += "<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; text-align: center; }</style>";
  page += "<style> body2 { background-color: #fffff; font-family: Georgia, Times, Serif; Color: #000088; text-align: center; }</style>";
  page += "<style> button { background-color: #195B6A; border: none; color: white; padding: 20px;";
  page += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
  page += ".button3 { background-color: #f70e0a; border: none; color: white; padding: 20px;";
  page += ".button2 {background-color: #77878A;}</style></head>";
  page += "</head><body><h1>AGV PROJECT</h1>";
  page += "</head><body2><h1>Hanoi University of Science and Technology</h1>";
  page += "</body></html>";
  Tvoltage = adc();
  page += "<p>Battery Voltage =" + (String)Tvoltage + " V</p>";
  if(Tvoltage<=1){
    page += "Battery is dead OR disconnected";
  }
  else if(Tvoltage>1 && Tvoltage<=3){
    page += "Need Imediate recharge";
  }
  else if(Tvoltage>3 && Tvoltage<=4.2){
    page += "Recharge";
  }
  else{
    page += "Battery Full";
  }
  page += "<p><a href=\"/START\"><button class=\"button\">START</button></a>";
  page += "<a href=\"/PAUSE\"><button class=\"button\">PAUSE</button></a>";
  page += "<a href=\"/READY\"><button class=\"button\">READY</button></a>";
  page += "<a href=\"/RESET\"><button class=\"button button3\">RESET</button></a></p>";
  page += "<p>TORCH - State " + outputTorchState + "</p>";     
  if (outputTorchState=="off") {
    page += "<p><a href=\"/TouchOn\"><button class=\"button\">ON</button></a></p>";
  }else if(outputTorchState=="on"){
    page += "<p><a href=\"/TouchOff\"><button class=\"button button2\">OFF</button></a></p>";
  }
  page += "<form action='/submit' method='POST'>";
  page += "<p>Speed: <INPUT type='number' name='speed' id='speed' value='' min='1' max='6'>";
  page += "<INPUT type='submit' value='Submit'></li></ul>";
  page += "<p>KP: <INPUT type='number' name='KP' id='KP' value='' min='0' max='1000'>";
  page += "<INPUT type='submit' value='Submit'></li></ul>";
  page += "<p>KI: <INPUT type='number' name='KI' id='KI' value='' min='0' max='1000'>";
  page += "<INPUT type='submit' value='Submit'></li></ul>";
  page += "<p>KD: <INPUT type='number' name='KD' id='KD' value='' min='0' max='1000'>";
  page += "<INPUT type='submit' value='Submit'></li></ul>";
  page += "<p>Powed by LamTran trandanhlam99@gmail.com</p>";
  return page;
}
void handleRoot(){
  server.send ( 200, "text/html", getPage() );
}
void handleLEDon() { 
  digitalWrite(outputTouch,HIGH);
  outputTorchState = "on";
  server.send ( 200, "text/html", getPage() );
}
void handleLEDoff() { 
  digitalWrite(outputTouch,LOW); 
  outputTorchState = "off";
  server.send ( 200, "text/html", getPage() );
}
void handleStart() { 
  digitalWrite(outputStart,HIGH); 
  delay(100);
  digitalWrite(outputStart,LOW);
  server.send ( 200, "text/html", getPage() );
}
void handlePause() { 
  digitalWrite(outputPause,HIGH); 
  delay(100);
  digitalWrite(outputPause,LOW);
  server.send ( 200, "text/html", getPage() );
}
void handleReady() { 
  digitalWrite(outputReady,HIGH); 
  delay(100);
  digitalWrite(outputReady,LOW);
  server.send ( 200, "text/html", getPage() );
}
void handleReset() { 
  Serial.println("RESET");
  Speed = "";
  KI = "";
  KP = "";
  KD = "";
  digitalWrite(outputTouch, LOW);
  outputTorchState = "off";
  digitalWrite(outputReset,HIGH); 
  delay(100);
  digitalWrite(outputReset,LOW);
  server.send ( 200, "text/html", getPage() );
}
void handleSubmit(){
  if (server.arg("speed")!= Speed && server.arg("speed")!= ""){
    Speed = server.arg("speed");
    Serial.println("Speed set: " + Speed);
    String s = " Speed set: " + Speed ;
    s += " <a href='/'> Go Back </a>";
    server.send(200, "text/html", s); 
  }
  else if (server.arg("KP")!= KP && server.arg("KP")!= ""){
    KP = server.arg("KP");
    Serial.println(" KP set: " + KP);
    String s = "KP set: " + KP ;
    s += " <a href='/'> Go Back </a>";
    server.send(200, "text/html", s); 
  }
  else if (server.arg("KI")!= KP && server.arg("KI")!= ""){
    KI = server.arg("KI");
    Serial.println(" KI set: " + KI);
    String s = "KI set: " + KI ;
    s += " <a href='/'> Go Back </a>";
    server.send(200, "text/html", s); 
  }
  else if (server.arg("KD")!= KD && server.arg("KD")!= ""){
    KD = server.arg("KD");
    Serial.println(" KD set: " + KD);
    String s = "KD set: " + KD ;
    s += " <a href='/'> Go Back </a>";
    server.send(200, "text/html", s); 
  }
}

void setup() {
  Serial.begin ( 115200 );
  // Initialize the output variables as outputs
  pinMode(outputStart, OUTPUT);
  pinMode(outputPause, OUTPUT);
  pinMode(outputReady, OUTPUT);
  pinMode(outputTouch, OUTPUT);
  pinMode(outputReset, OUTPUT);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(sonar, OUTPUT);
  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(getIP, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(getIP), GetIP, FALLING);

  // Set outputs to LOW
  digitalWrite(outputStart, LOW);
  digitalWrite(outputPause, LOW);
  digitalWrite(outputReady, LOW);
  digitalWrite(outputTouch, LOW);
  digitalWrite(outputReset, LOW);
  
  WiFi.begin ( ssid, password );  
  // Waiting for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ); Serial.print ( "." );
  }
  //WiFi connection is OK
  Serial.println ( "" );
  Serial.print ( "Connected to " ); Serial.println ( ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );

  WiFi.softAP(host_ssid, host_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);

  // Link to the function that manage launch page

  server.on("/", handleRoot);
  server.on("/submit", handleSubmit);
  server.on("/TouchOn", handleLEDon);
  server.on("/TouchOff", handleLEDoff);
  server.on("/START", handleStart);
  server.on("/PAUSE", handlePause);
  server.on("/READY", handleReady);
  server.on("/RESET", handleReset);
  server.begin();
  Serial.println ( "HTTP server started" );
 
}
//-------------------Interrupt-----------------------//
 ICACHE_RAM_ATTR void GetIP() {
  Serial.println(WiFi.localIP());
}
//-----------------------Battery Voltage-----------------------//
float adc(){
  int value = LOW;
  float Tvoltage=0.0;
  float Vvalue=0.0,Rvalue=0.0;
  for(unsigned int i=0;i<10;i++){
    Vvalue=Vvalue+analogRead(BAT);         //Read analog Voltage
    delay(5);                              //ADC stable
   }
  Vvalue=(float)Vvalue/10.0;            //Find average of 10 values
  Rvalue=(float)(Vvalue/1024.0)*5;      //Convert Voltage in 5v factor
  Tvoltage=Rvalue*RatioFactor;          //Find original voltage by multiplying with factor
  return Tvoltage;
 }


void loop() {
  server.handleClient();
    /*sonar sensor*/
  long duration, distance;
  digitalWrite(TRIGGER, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;
  if (distance < 15){
    digitalWrite(sonar, LOW);
  }
  else{
    digitalWrite(sonar, HIGH);  
  }
  
}
