#include <WiFi.h>

const char* ssid     = "Vodafone-BC8D";
const char* password = "T8hHQQCFQrpLMgGb";
bool bStart = false;

WiFiServer server(80);

void HandleClient(){
    bStart = true;
    server.send(200, "text/plan", "OK");
}

server.on("/Start", HandleClient());

