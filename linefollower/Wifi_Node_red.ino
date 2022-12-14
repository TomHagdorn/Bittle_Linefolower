/*
 WiFi Web Server 
 */

#include <WiFi.h>

const char* ssid     = "Vodafone-BC8D";
const char* password = "T8hHQQCFQrpLMgGb";


WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(5, OUTPUT);      // set the LED pin mode

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

int value = 0;

void loop(){
 bool status ; 
 WiFiClient client = server.available();   // listen for incoming clients
 
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            // client.println("HTTP/1.1 200 OK");
            // client.println("Content-type:text/html");
            // client.println();

            // the content of the HTTP response follows the header:
             client.print("Verbindung steht !!!");
            client.print("Click <a href=\"S\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
        // status in bool
         
        // Check to see if the client request was "GET Start and Stop":
        if (currentLine.endsWith("GET /S")) {
          digitalWrite(status, true);   
          Serial.println(status);
          client.println(status);           // GET Start to change the value
        }
        if (currentLine.endsWith("GET /X")) {
          status = false;    
          Serial.println(status);            // GET Stop to change the value
        }
        
      }
    }
    // close the connection:
    Serial.println(status);
    client.stop();
    
    Serial.println("Client Disconnected.");
  }
}
/*
 WiFi Web Server 
 */

#include <WiFi.h>

const char* ssid     = "Vodafone-BC8D";
const char* password = "T8hHQQCFQrpLMgGb";


WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(5, OUTPUT);      // set the LED pin mode

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

int value = 0;

void loop(){
 bool status ; 
 WiFiClient client = server.available();   // listen for incoming clients
 
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            // client.println("HTTP/1.1 200 OK");
            // client.println("Content-type:text/html");
            // client.println();

            // the content of the HTTP response follows the header:
             client.print("Verbindung steht !!!");
            client.print("Click <a href=\"S\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
        // status in bool
         
        // Check to see if the client request was "GET Start and Stop":
        if (currentLine.endsWith("GET /S")) {
          digitalWrite(status, true);   
          Serial.println(status);
          client.println(status);           // GET Start to change the value
        }
        if (currentLine.endsWith("GET /X")) {
          status = false;    
          Serial.println(status);            // GET Stop to change the value
        }
        
      }
    }
    // close the connection:
    Serial.println(status);
    client.stop();
    
    Serial.println("Client Disconnected.");
  }
}