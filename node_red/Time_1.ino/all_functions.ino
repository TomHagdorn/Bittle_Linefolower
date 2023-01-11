#include <WiFi.h>
#include <WebServer.h>
// ADD to linefollower.ino
#include <esp_system.h>


const char *ssid = "Get off my Lan!";
const char *password = "prettyflyforAWifi";

int Treshold_value = 1;
int recover_time  = 1;
int Obst_left_t =1;
int Obst_straight_t=1;
int Obst_right_t =1;
int Line_width=1;
int Fin_line_width=1;
int currentlinewidth = 1;
int currentfinlinewidth =1;
int lastServerCheck = 0;

//ADD to linefollower.ino
IPAddress staticIP(192, 168, 1, 5);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0); 

WebServer server(80);

void setup() {
  Serial.begin(115200);

  //This part needs to be at the bottom of the setup function in linefollower.ino
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);
  //Blocks further code until connected
  int connRes = WiFi.waitForConnectResult();
  if (connRes == WL_CONNECTED) {
    Serial.print("Connected to WiFi network with IP: ");
    Serial.println(WiFi.localIP());
    server.begin();
    Change_Treshold_value();
  } else {
    Serial.println("Connection Failed!");
    esp_restart();
    return;
  }
  Serial.println(WiFi.localIP());
  server.begin();
  Change_Treshold_value();
}

void loop() {
    server.handleClient();
    if ((unsigned long)(millis() - lastServerCheck) >= 1000UL)
    {
        if (WiFi.isConnected()) {
            Change_Treshold_value();
            Serial.println(Treshold_value);
            //Serial.println(WiFi.localIP());
            
        } else {
            Serial.println("WiFi not connected.");
        }
      lastServerCheck = millis();
    }
    
}

void Change_Treshold_value(){
    server.on("/update-tresholdvalue", HTTP_GET, []() {
    String newValue = server.arg("value");
    Treshold_value = newValue.toInt();
  });
}