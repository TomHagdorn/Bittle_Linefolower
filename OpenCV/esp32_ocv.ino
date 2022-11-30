#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>
 
const char* WIFI_SSID = "ssid";
const char* WIFI_PASS = "password";
 
WebServer server(80);
 
 
static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);
void serveJpg()
{
  auto frame = esp32cam::capture(); // capture a frame
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL"); // capture failed
    server.send(503, "", "");       // 503 Service Unavailable
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size()));
 
  server.setContentLength(frame->size());       // set the content length
  server.send(200, "image/jpeg");            // send the header
  WiFiClient client = server.client();      // get the client
  frame->writeTo(client);
}
 
void handleJpgLo()   // low resolution
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgMid()     // 350x530
{
  if (!esp32cam::Camera.changeResolution(midRes)) {     
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}
 
void  setup(){
  Serial.begin(115200); // start serial
  Serial.println(); // print a newline
  {
    using namespace esp32cam;
    Config cfg; // create a config
    cfg.setPins(pins::AiThinker);   // set the pins
    cfg.setResolution(hiRes);    // set the resolution
    cfg.setBufferCount(2);     // set the buffer count
    cfg.setJpeg(80);       // set the jpeg quality
 
    bool ok = Camera.begin(cfg);        // initialize the camera
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  WiFi.persistent(false);   // disable wifi persistence
  WiFi.mode(WIFI_STA);    // set wifi mode to station
  WiFi.begin(WIFI_SSID, WIFI_PASS); // connect to wifi
  while (WiFi.status() != WL_CONNECTED) {   // wait for connection
    delay(500);     // wait 500ms #TODO add millis timeout
  }
  Serial.print("http://");      // print the IP address
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");  // print the paths
  Serial.println("  /cam-hi.jpg");      
  Serial.println("  /cam-mid.jpg");
 
  server.on("/cam-lo.jpg", handleJpgLo);    // register handleJpgLo to handle the request
  server.on("/cam-hi.jpg", handleJpgHi);    // register handleJpgHi to handle the request
  server.on("/cam-mid.jpg", handleJpgMid);  // register handleJpgMid to handle the request
 
  server.begin();                         // start the HTTP server
}
 
void loop()
{
  server.handleClient();              // handle client requests
}