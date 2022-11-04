#include <SoftwareSerial.h>
#define DEBUG true
#define Led_Pin 13
 
SoftwareSerial esp8266(2,3);
char x;


void setup() {
  Serial.begin(115200);
  esp8266.begin(115200); // chu y phai cung toc do voi ESP 8266
 
  pinMode(Led_Pin, OUTPUT);
  digitalWrite(Led_Pin, LOW);
  //==================gui tap lenh AT cho ESP 8266 de cai dat cac tham so cho WiFi=======//
  guidulieu("AT+RST\r\n", 2000, DEBUG); // reset module
  guidulieu("AT+CWMODE=1\r\n", 2000, DEBUG); // Chọn chức năng client cho esp
  guidulieu("AT+CWJAP=\"Tang2_NhaA1\",\"@tu0den8\"\r\n", 10000, DEBUG); // Kết nối với wifi 
  guidulieu("AT+CIPSTA=”192.168.0.178”\r\n", 5000, DEBUG); // xem ip là bn
  //guidulieu("AT+CWLIF\r\n", 1000, DEBUG); fix ip
  guidulieu("AT+CIPMUX=1\r\n", 2000, DEBUG); // configure for multiple connections
  guidulieu("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // Mở cổng 80
}
 
void loop() {
  while (esp8266.available()) {
    IncomingChar(esp8266.read());
  }
 
  if (x) {
    // Đây là giao diện web page viết dưới dạng html...các bạn có thể dễ dàng edit
 
    String html = "<HTML>"
    "<html>"
      "<head>"
        "<title>E360 ESP8266 DEMO</title>"
      "</head>"
      "<body>"
        "<h1>E360 ESP8266 DEMO</h1>"
        "<form action='' method='GET'>"
          "<input type='radio' name='LED' value='RED_ON' /> LED ON<br/>"
          "<input type='radio' name='LED' value='RED_OFF' /> LED OFF<br/>"
          "<input type='submit' value='Submit' />"
        "</form>"
      "</body>"
     "</html>";
    String cipsend = "AT+CIPSEND=0,"; //gui chuoi data qua Wifi
 
    cipsend += html.length();
    cipsend += "\r\n";
    guidulieu(cipsend, 1000, DEBUG);
    guidulieu(html, 1000, DEBUG);
    guidulieu("AT+CIPCLOSE=0\r\n", 1000, DEBUG);
    x = 0;
  }
 
}
 
void IncomingChar(const byte InChar) {
  static char InLine[300]; //Hạn chế ký tự
  static unsigned int Position = 0;
 
  switch (InChar) {
  case '\r': //Cái này là xuống dòng...cho linux
    break;
 
  case '\n': // xuống dòng cho window...( kết thúc bảng tin )
    InLine[Position] = 0;
    ProcessCommand(String(InLine));
    Position = 0;
    break;
 
  default:
    InLine[Position++] = InChar;
 
  }
}
 
void ProcessCommand(String InLine) {
  Serial.println("InLine: " + InLine);
 
  if (InLine.startsWith("+IPD,")) {
 
    x = 1;
  }
  // lện String.indexOf(kytu)...chả về vị trí của kytu trong chuỗi String...Nếu giá trị trả về là -1...tức là kytu không xuất hiện trong chuỗi String
  if (InLine.indexOf("RED_OFF") != -1) {
 
    digitalWrite(Led_Pin, LOW);
  }
 
  if (InLine.indexOf("RED_ON") != -1) {
 
    digitalWrite(Led_Pin, HIGH);
  }
}
 
String guidulieu(String lenh,
  const int thoigian, boolean debug) {
  String chuoigiatri = "";
 
  esp8266.print(lenh); // send the read character to the esp8266
 
  long int time = millis();
 
  while ((time + thoigian) > millis()) {
    while (esp8266.available()) {
 
      // The esp has data so display its output to the serial window 
      char c = esp8266.read(); // read the next character.
      chuoigiatri += c;
    }
  }
 
  if (debug) {
    Serial.print(chuoigiatri);
  }
 
return chuoigiatri;
}
