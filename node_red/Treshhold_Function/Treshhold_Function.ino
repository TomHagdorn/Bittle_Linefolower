#include <WiFi.h>
#include <WebServer.h>


const char* ssid     = "Vodafone-BC8D";
const char* password = "T8hHQQCFQrpLMgGb";
int teshold_Value = 1; // the variable that you want to update

WebServer server(80);

void setup() {
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  server.begin();
  Change_Value();
}

void loop() {

  server.handleClient();
  Serial.println(teshold_Value);

}

void Change_Value(){
    server.on("/update-variable", HTTP_GET, []() {
    String newValue = server.arg("value");
    teshold_Value = newValue.toInt();
  });
}