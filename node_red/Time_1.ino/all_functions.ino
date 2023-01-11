#include <WiFi.h>
#include <WebServer.h>


const char* ssid     = "Vodafone-BC8D";
const char* password = "T8hHQQCFQrpLMgGb";

int Treshold_value = 1;
int recover_time  = 1;
int Obst_left_t =1;
int Obst_straight_t=1;
int Obst_right_t =1;
int Line_width=1;
int Fin_line_width=1;
int currentlinewidth = 1;
int currentfinlinewidth =1;
 // the variable that you want to update

WebServer server(80);

void setup() {
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println(WiFi.localIP());
  server.begin();
  Change_Treshold_value();
}

void loop() {

  server.handleClient();
  Serial.println(Treshold_value);

}

void Change_Treshold_value(){
    server.on("/update-tresholdvalue", HTTP_GET, []() {
    String newValue = server.arg("value");
    Treshold_value = newValue.toInt();
  });
}
// void Change_recover_time (){
//     server.on("/update-recovertime ", HTTP_GET, []() {
//     String newValue = server.arg("value");
//     recover_time  = newValue.toInt();
//   });
// }
// void Change_Obst_left_t (){
//     server.on("/update-obstleftt ", HTTP_GET, []() {
//     String newValue = server.arg("value");
//     Obst_left_t  = newValue.toInt();
//   });
// }
// void Change_Obst_straight_t(){
//     server.on("/update-obststraightt", HTTP_GET, []() {
//     String newValue = server.arg("value");
//     Obst_straight_t = newValue.toInt();
//   });
// }
// void Change_Obst_right_t (){
//     server.on("/update-obstrightt ", HTTP_GET, []() {
//     String newValue = server.arg("value");
//     Obst_right_t  = newValue.toInt();
//   });
// }
// void Change_Line_width(){
//     server.on("/update-linewidth", HTTP_GET, []() {
//     String newValue = server.arg("value");
//     Line_width = newValue.toInt();
//   });
// }
// void Change_Fin_line_width(){
//     server.on("/update-finlinewidth", HTTP_GET, []() {
//     String newValue = server.arg("value");
//     Fin_line_width = newValue.toInt();
//   });
// }
// void Change_currentlinewidth(){
//     server.on("/update-currentlinewidth", HTTP_GET, []() {
//     String newValue = server.arg("value");
//     currentlinewidth = newValue.toInt();
//   });
// }
// void Change_currentfinlinewidth(){
//     server.on("/update-currentfinlinewidth", HTTP_GET, []() {
//     String newValue = server.arg("value");
//     currentfinlinewidth = newValue.toInt();
//   });
// }