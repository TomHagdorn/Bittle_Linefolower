#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// Set your Static IP address
// IPAddress local_IP(192, 168, 1, 184);
// // Set your Gateway IP address
// IPAddress gateway(192, 168, 1, 1);



bool server_on = true;
bool server_status = false;

//TODO Set server to lower power consumption by running it less frequent.
WebServer server(80);

/**
  * @brief Setup the wifi connection
  * 
  */
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
    IPAddress myIP = WiFi.softAPIP();
    Serial.printf("IP   - %s\n", myIP.toString());
    Serial.println("Connecting to WiFi...");
    }

    
void HandleClienttrue(){
    bstart = true;
    server.send(200, "text/plan", "OK");
}

void HandleClientfalse(){
    bstart = false;
    server.send(200, "text/plan", "OK");
}


/**
* @brief change threshold value
* 
*/
void Change_Treshold_value(){
    server.on("/update-tresholdvalue", HTTP_GET, []() {
    String newValue = server.arg("value");
    pixel_threshold = newValue.toInt();
  });
}

/**
* @brief change gain value
* 
*/
void Change_IMG_Gain_value(){
    server.on("/update-gain", HTTP_GET, []() {
    String newValue = server.arg("value");
   cameraImageGain = newValue.toInt();
   cameraImageSettings();
  });
}
/**
  *  @brief change exposure value
  * 
*/
void Change_IMG_Exposur_value(){
    server.on("/update-exposure", HTTP_GET, []() {
    String newValue = server.arg("value");
   cameraImageExposure = newValue.toInt();
   cameraImageSettings();
  });
}

/**
  *  @brief change obstacle detection distance
  * 
*/
void Change_Obstacle_Distance(){
    server.on("/update-obstacle-distance", HTTP_GET, []() {
    String newValue = server.arg("value");
   obstacle_detection_dist = newValue.toInt();
  });
}

/**
  *  @brief change obstacle detection distance tolerance
  * 
*/
void Change_Obstacle_Tollerance(){
    server.on("/update-obstacle-tollerance", HTTP_GET, []() {
    String newValue = server.arg("value");
   obstacle_tolerance = newValue.toInt();
  });
}

/**
 * @brief send the image to the server
*/
void send_image() {
    // server will do the following every time [esp32-ip]/image is requested:
    server.on(F("/image"), [&]() {
        // convert frame to bmp
        uint8_t * buf = NULL;
        size_t buf_length = 0;
        frame2bmp(fb, &buf, &buf_length);
        // send image
        server.send_P(200, "image/bmp", (const char *)buf, buf_length);
        // free memory and return buffer
        // note that the picture is actually taken when you return the frame buffer
        free(buf);
    });
}



/*void update_server()

{
    if (server_on == true && server_status == false){
      setup_server();
      server_status = true;
    }
    if (server_on == false){
        //TODO end server
        WiFi.softAPdisconnect(true);
        server_status = false;
    }
    //check if the server is connected
    if (server_status = true && server_on == true)
    {
        server.handleClient();
    }
}


*/

/**
 * @brief update the status of the robot displayed on node-red dashboard
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
void Update_node_red_values(){
  Change_Treshold_value();
  Change_IMG_Gain_value();
  Change_IMG_Exposur_value();
  Change_Obstacle_Tollerance();
  Change_Obstacle_Distance();
}
