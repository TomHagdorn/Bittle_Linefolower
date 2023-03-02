#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>



#define SSID "esp32_server"
#define PWD "123456789"


const char* ssid = "Vodafone-BC8D";
const char* password = "T8hHQQCFQrpLMgGb";


int pixel_threshold = 170; // the variable that you want to update
bool server_on = true;
bool server_status = false;


WebServer server(80);

void setup_wifi() {
    Serial.println("Configuring AP...");
    if (!WiFi.softAP(SSID, PWD)) Serial.println("AP Config failed.");

    // print ssid and pwd
    Serial.printf("SSID - %s\n", SSID);
    Serial.printf("PWD  - %s\n", PWD);
}



void Change_Treshold_value(){
    server.on("/update-tresholdvalue", HTTP_GET, []() {
    String newValue = server.arg("value");
    pixel_threshold = newValue.toInt();
  });
}

void setup_server() {
    // server will do the following every time [esp32-ip]/image is requested:
    server.on(F("/image"), [&]() {
        // send message on serial for debugging
        // take picture
        //camera_fb_t * fb = esp_camera_fb_get();
        //threshold_image(fb, pixel_threshold);


        // convert frame to bmp
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

    setup_wifi();

    WiFi.begin(ssid, password);
    int connRes = WiFi.waitForConnectResult();
    if (connRes == WL_CONNECTED) {
        //Serial.print("Connected to WiFi network with IP: ");
        Serial.println(WiFi.localIP());
        
        //Call functions for the variable nodered values
        //Change_Treshold_value();
    } else {
        Serial.println("Connection Failed!");
        esp_restart();
        return;
    }


    // start server
    server.begin();
    // print ip
    IPAddress myIP = WiFi.softAPIP();
    Serial.printf("IP   - %s\n", myIP.toString());
}





void status_send() {
  server.send(200, "text/plain", "laufe rechts");

void update_server()
{
    if (server_on == true && server_status == false){
      setup_server();
      server_status = true;
    }
    else{
        //TODO end server and wifi
        server_status = false;
    }
    //check if the server is connected
    if (server_status = true)
    {
        server.handleClient();
    }
}

/*
void status_send_rechts() {
  server.send(200, "text/plain", "rechts");
}
void status_send_links() {
  server.send(200, "text/plain", "links");
}
void status_send_balance() {
  server.send(200, "text/plain", "balance");
}
void status_send_vorne() {
  server.send(200, "text/plain", "vorne");
}*/

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

