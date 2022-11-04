#include <ESP8266WiFi.h>
#include <EEPROM.h>

// Replace with your network credentials
const char* ssid     = "VIETTEL_CHUYEN";
const char* password = "0903482723";

// EEPROM size
// Address 0: Speed
// Address 1: KD
// Address 2: KI
// Address 3: KP
#define EEPROM_SIZE 4

// Timers - Auxiliary variables
unsigned long now = millis();
unsigned long lastMeasure = 0;
boolean startTimer = false;
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
// Auxiliary variables to store selected mode and settings 
int Speed = 0;
int KD = 0;
int KP = 0;
int KI = 0;
// Decode HTTP GET value
String valueString = "0";
int pos1 = 0;
int pos2 = 0;
// Variable to store the HTTP request
String header;
// Set web server port number to 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
    Serial.println("start...");
  EEPROM.begin(EEPROM_SIZE);
    Serial.println("start...");
  EEPROM.begin(EEPROM_SIZE);
  Speed = EEPROM.read(0);
  KD = EEPROM.read(1);
  KI = EEPROM.read(2);
  KP = EEPROM.read(3);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
void loop(){
WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {            // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();                     
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            // Web Page Heading
            client.println("<body><h1>AGV By Lam Tran</h1>");
            client.println("<body><h1>DHBKHN</h1>");

             // Request example: GET /?mode=0& HTTP/1.1 - sets mode to Manual (0)
            if(header.indexOf("GET /?Speed=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              Speed = valueString.toInt();
              EEPROM.write(0, Speed);
              EEPROM.commit();
              Serial.println(valueString);
            }
             if(header.indexOf("GET /?KD=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              KD = valueString.toInt();
              EEPROM.write(1, KD);
              EEPROM.commit();
              Serial.println(valueString);
            }
             if(header.indexOf("GET /?KI=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              KI = valueString.toInt();
              EEPROM.write(2, KI);
              EEPROM.commit();
              Serial.println(valueString);
            }
             if(header.indexOf("GET /?KP=") >= 0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              KP = valueString.toInt();
              EEPROM.write(3, KP);
              EEPROM.commit();
              Serial.println(valueString);
            }
            
            client.println("<p>Speed: <input type=\"number\" name=\"txt\" value=\"" + 
                        String(EEPROM.read(0)) + "\" onchange=\"setSpeed(this.value)\" min=\"50\" max=\"5000\"></p>");
                        client.println("<p>KD: <input type=\"number\" name=\"txt\" value=\"" + 
                        String(EEPROM.read(1)) + "\" onchange=\"setKD(this.value)\" min=\"0\" max=\"1000\"></p>");
                        client.println("<p>KI: <input type=\"number\" name=\"txt\" value=\"" + 
                        String(EEPROM.read(2)) + "\" onchange=\"setKI(this.value)\" min=\"0\" max=\"1000\"></p>");
                        client.println("<p>KP: <input type=\"number\" name=\"txt\" value=\"" + 
                        String(EEPROM.read(3)) + "\" onchange=\"setKP(this.value)\" min=\"0\" max=\"1000\"></p>");
           // client.println("</body></html>");

            client.println("<script> function setSpeed(value) { var xhr = new XMLHttpRequest();"); 
            client.println("xhr.open('GET', \"/?Speed=\" + value + \"&\", true);"); 
            client.println("xhr.send(); location.reload(true); } ");
            client.println("function setKD(value) { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?KD=\" + value + \"&\", true);"); 
            client.println("xhr.send(); location.reload(true); } ");
            client.println("function setKI(value) { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?KI=\" + value + \"&\", true);"); 
            client.println("xhr.send(); location.reload(true); } ");
            client.println("function setKP(value) { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?KP=\" + value + \"&\", true);"); 
            client.println("xhr.send(); location.reload(true); }</script></body></html> ");
            /*client.println("function outputOn() { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?state=on\", true);"); 
            client.println("xhr.send(); location.reload(true); } ");
            client.println("function outputOff() { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?state=off\", true);"); 
            client.println("xhr.send(); location.reload(true); } ");
            client.println("function updateSensorReadings() { var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/?sensor\", true);"); 
            client.println("xhr.send(); location.reload(true); }</script></body></html>");*/
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
