#define LED_PIN 13
 
#define CMD_SEND_BEGIN  "AT+CIPSEND=0"
#define CMD_SEND_END    "AT+CIPCLOSE=0"
 
#define E360_PROTOCOL_HTTP     80
#define E360_PROTOCOL_HTTPS    443
#define E360_PROTOCOL_FTP      21
#define E360_PROTOCOL_CURRENT  E360_PROTOCOL_HTTP
 
#define E360_CHAR_CR     0x0D
#define E360_CHAR_LF     0x0A
 
#define E360_STRING_EMPTY  ""
 
#define E360_DELAY_SEED  1000
#define E360_DELAY_1X    (1*E360_DELAY_SEED)
#define E360_DELAY_2X    (2*E360_DELAY_SEED)
#define E360_DELAY_3X    (3*E360_DELAY_SEED)
#define E360_DELAY_4X    (4*E360_DELAY_SEED)
#define E360_DELAY_0X    (5*E360_DELAY_SEED)
 
bool hasRequest = false;
 
void setup()
{
  delay(E360_DELAY_0X);
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, LOW);
  
  initESP8266();
}
 
void loop()
{
  while(Serial.available())
  {   
    bufferingRequest(Serial.read());
  }
  
  if(hasRequest == true) 
  {
    String htmlResponse = "<!doctype html>"
    "<html>"
      "<head>"
        "<title>E360 ESP8266 DEMO</title>"
      "</head>"
      "<body>"
        "<h1>E360 ESP8266 DEMO</h1>"
        "<form action='' method='GET'>"
          "<input type='radio' name='LED' name='E360' value='E360_ON' /> LED ON<br/>"
          "<input type='radio' name='LED' name='D2K' value='E360_OFF' /> LED OFF<br/>"
          "<input type='submit' value='Submit' />"
        "</form>"
      "</body>"
     "</html>";
    
    String beginSendCmd = String(CMD_SEND_BEGIN) + "," + htmlResponse.length();
    deliverMessage(beginSendCmd, E360_DELAY_1X);
    deliverMessage(htmlResponse, E360_DELAY_1X);
    deliverMessage(CMD_SEND_END, E360_DELAY_1X);
    hasRequest = false;
  }
}
 
void initESP8266()
{
  deliverMessage("AT+RST", E360_DELAY_2X);
  deliverMessage("AT+CWMODE=2", E360_DELAY_3X);
  deliverMessage("AT+CWSAP=\"TuHocArduino\",\"123456789\",1,4", E360_DELAY_3X);
  deliverMessage("AT+CIFSR", E360_DELAY_1X);
  deliverMessage("AT+CIPMUX=1", E360_DELAY_1X); // để cho phép các kết nối TCP
  deliverMessage(String("AT+CIPSERVER=1,") + E360_PROTOCOL_CURRENT, E360_DELAY_1X);  //để tạo 1 TCP server
}
 
void bufferingRequest(char c)
{
  static String bufferData = E360_STRING_EMPTY;
 
  switch (c)
  {
    case E360_CHAR_CR:
      break;
    case E360_CHAR_LF:
    {
      E360Procedure(bufferData);
      bufferData = E360_STRING_EMPTY;
    }
      break;
    default:
      bufferData += c;
  }
} 
 
void E360Procedure(const String& command)
{ 
  hasRequest = command.startsWith("+IPD,");
  
  if(command.indexOf("E360_OFF") != -1)
  { 
    digitalWrite(LED_PIN, LOW);
  }
  else if(command.indexOf("E360_ON") != -1)
  { 
    digitalWrite(LED_PIN, HIGH);
  }
}
 
void deliverMessage(const String& msg, int dt)
{
  Serial.println(msg);
  delay(dt);

}
