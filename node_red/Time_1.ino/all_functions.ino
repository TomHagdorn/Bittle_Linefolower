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
int Line_width=22;
int Fin_line_width=1;
int currentlinewidth = 1;
int currentfinlinewidth =1;
int lastServerCheck = 0;

//ADD to linefollower.ino
// IPAddress staticIP(192, 168, 0, 43);
// IPAddress gateway(192, 168, 0, 1);
// IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

void setup() {
  Serial.begin(115200);

  //This part needs to be at the bottom of the setup function in linefollower.ino
  //WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);
  //Blocks further code until connected
  int connRes = WiFi.waitForConnectResult();
  if (connRes == WL_CONNECTED) {
    Serial.print("Connected to WiFi network with IP: ");
    Serial.println(WiFi.localIP());
    server.begin();
    //Change_Line_width();
    Change_Treshold_value();
  
  } else {
    Serial.println("Connection Failed!");
    esp_restart();
    return;
  }

  
 
}

void loop() {
    server.handleClient();
     
    if ((unsigned long)(millis() - lastServerCheck) >= 1000UL)
    {
        if (WiFi.isConnected()) {
            Serial.println(Treshold_value);
            //Serial.println(WiFi.localIP());
            Serial.println(Line_width);
        } else {
            Serial.println("WiFi not connected.");
        }
      lastServerCheck = millis();
    }
    if (Line_width==0){
      
      Change_Treshold_value();
    }
    
}

void Change_Treshold_value(){
    server.on("/update-tresholdvalue", HTTP_GET, []() {
    String newValue = server.arg("value");
    Treshold_value = newValue.toInt();
  });
}

void Change_Line_width(){
    server.on("/update-linewidth", HTTP_GET, []() {
    String newValue = server.arg("value");
    Line_width = newValue.toInt();
  });
}