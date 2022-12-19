#include <WiFi.h>

const char* ssid     = "Vodafone-BC8D";
const char* password = "T8hHQQCFQrpLMgGb";

WiFiServer server(80);

void setup()
{
     Serial.begin(115200);
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
bool status = false;

void loop(){
 WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {



          if (currentLine.length() == 0) {
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }


        if (currentLine.endsWith("GET /S")) {
          status = true; 


        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}