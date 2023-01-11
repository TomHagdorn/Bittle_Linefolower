  #include <WiFi.h>
  #include <WebServer.h>


  const char* ssid     = "Vodafone-BC8D";
  const char* password = "T8hHQQCFQrpLMgGb";
  bool bStart = false;

  WebServer server(80);

  void setup() {
  
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    Serial.println(WiFi.localIP());
    server.begin();

    server.on("/Start", HandleClienttrue); // Wifi functions to start or stop the update()
    server.on("/Stop", HandleClientfalse);
    
  }

  void loop() {

    server.handleClient();
    Serial.println(bStart);

  }

void HandleClienttrue(){
    bStart = true;
    server.send(200, "text/plan", "OK");
}

void HandleClientfalse(){
    bStart = false;
    server.send(200, "text/plan", "OK");
}