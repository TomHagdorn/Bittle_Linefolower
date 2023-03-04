#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>


const char* ssid = "Alpha_Lan";
const char* password = "david123";


 // the variable that you want to update
bool server_on = false;
bool server_status = false;



WebServer server(80);

void setup_wifi() {
  
    WiFi.begin(ssid, password);
    int connRes = WiFi.waitForConnectResult();
    if (connRes == WL_CONNECTED) {
        //Serial.print("Connected to WiFi network with IP: ");
        Serial.println(WiFi.localIP());
        //server.begin();
        

    } else {
        Serial.println("Connection Failed!");
        esp_restart();
        return;
    }

    server.begin();
    }

    

void Change_Treshold_value(){
    server.on("/update-tresholdvalue", HTTP_GET, []() {
    String newValue = server.arg("value");
    pixel_threshold = newValue.toInt();
  });
}


void send_image() {

  if(server_on = true){
    // server will do the following every time [esp32-ip]/image is requested:
    server.on(F("/image"), [&]() {
        // convert frame to bmp
        Serial.println("sende Bild!!!");
        uint8_t * buf = NULL;
        size_t buf_length = 0;
        frame2bmp(fb, &buf, &buf_length);
        // send image
        server.send_P(200, "image/bmp", (const char *)buf, buf_length);
        // free memory and return buffer
        // note that the picture is actually taken when you return the frame buffer
        free(buf);
        //esp_camera_fb_return(fb);
    });
    }
    }
  //TODO Make own wifi setup function


void image_stop(){
  server_on = false;
  server.send(200, "text/plain", "Stop");
}
void image_start(){
  server_on = true;
  server.send(200,"text/plain","Start");
}



/*void update_server()

{
    if (server_on == true && server_status == false){
      send_image();
      server_status = true;
    }
    if (server_on == false){
        //TODO end server
        server_status = false;
    }
    if (server_on = false && server_status == true){
      server.handleClient();
    }
    //check if the server is connected
}
*/




void handle_status() {
  
  if (currentMovementState == STATE_STOP) {
    server.send(200, "text/plain", "Stop");
  }
  if (currentMovementState == STATE_TURN_LEFT) {
    server.send(200, "text/plain", "TURN_LEFT");
  }
  if (currentMovementState == STATE_TURN_RIGHT) {
    server.send(200, "text/plain", "TURN_RIGHT");
  } 
  if (currentMovementState == STATE_MOVE_FORWARD) {
    server.send(200, "text/plain", "MOVE_FORWARD");
  } 
  if (currentMovementState == STATE_MOVE_BACKWARD) {
    server.send(200, "text/plain", "MOVE_BACKWARDS");
  }
  if (currentMovementState == STATE_TURN_BACK_RIGHT) {
    server.send(200, "text/plain", "TURN_BACK_RIGHT");
  }
  if (currentMovementState == STATE_TURN_BACK_LEFT) {
    server.send(200, "text/plain", "TURN_BACK_LEFT");
  }
  else{
    server.send(200, "text/plain", "KEIN_STATE");
  }
}





/*
void Change_recover_time (){
    server.on("/update-recovertime ", HTTP_GET, []() {
    String newValue = server.arg("value");
    recover_time  = newValue.toInt();
  });
}
void Change_Obst_left_t (){
    server.on("/update-obstleftt ", HTTP_GET, []() {
    String newValue = server.arg("value");
    Obst_left_t  = newValue.toInt();
  });
}
void Change_Obst_straight_t(){
    server.on("/update-obststraightt", HTTP_GET, []() {
    String newValue = server.arg("value");
    Obst_straight_t = newValue.toInt();
  });
}
void Change_Obst_right_t (){
    server.on("/update-obstrightt ", HTTP_GET, []() {
    String newValue = server.arg("value");
    Obst_right_t  = newValue.toInt();
  });
}
void Change_Line_width(){
    server.on("/update-linewidth", HTTP_GET, []() {
    String newValue = server.arg("value");
    Line_width = newValue.toInt();
  });
}
void Change_Fin_line_width(){
    server.on("/update-finlinewidth", HTTP_GET, []() {
    String newValue = server.arg("value");
    Fin_line_width = newValue.toInt();
  });
}
void Change_currentlinewidth(){
    server.on("/update-currentlinewidth", HTTP_GET, []() {
    String newValue = server.arg("value");
    currentlinewidth = newValue.toInt();
  });
}
void Change_currentfinlinewidth(){
    server.on("/update-currentfinlinewidth", HTTP_GET, []() {
    String newValue = server.arg("value");
    currentfinlinewidth = newValue.toInt();
  });
}*/

//TODO add states to node red
//TODO add movement states to node red
//TODO add new node redvariables to node red