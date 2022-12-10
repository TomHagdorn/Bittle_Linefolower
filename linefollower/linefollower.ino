#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

//Depenencies for image processing
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>

// Constants for the dimensions of the image
const int IMAGE_WIDTH = 256;
const int IMAGE_HEIGHT = 256;

// The image data, stored as a 2D array of pixels
int image[IMAGE_WIDTH][IMAGE_HEIGHT];

int selectedRegion = 0;

// Replace with your own Wi-Fi credentials
const char* ssid = "your_network_name";
const char* password = "your_network_password";

// Replace with your own API URL and port
const char* host = "your_api_url";
const int port = 80; // or whichever port your API is running on

// Function to send a command to the API
void sendCommand(String command, String parameters) {
  // Create an HTTP client
  HTTPClient http;

  // Create a JSON object for the command
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["command"] = command;
  jsonDoc["parameters"] = parameters;

  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Send a POST request to the API with the JSON command
  http.begin(host, port, "/your_api_endpoint");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(jsonString);

  // Check if the request was successful
  if (httpCode == HTTP_CODE_OK) {
    // TODO: Add code here to handle the response from the API
  } else {
    Serial.println("Error sending POST request: " + httpCode);
  }

  // Close the connection
  http.end();
}


// Function that splits the image into 3 horizontal regions and returns a vector
// containing the y-coordinates of the split points.
std::vector<int> splitImage()
{
    // The size of each region is 1/3 of the image height
    int regionSize = IMAGE_HEIGHT / 3;

    // The vector that will be returned, containing the y-coordinates
    // of the split points.
    std::vector<int> splitPoints;

    // The first split point is at the top of the image
    splitPoints.push_back(0);

    // The next two split points are at the middle and bottom of the
    // first and second regions, respectively.
    splitPoints.push_back(regionSize);
    splitPoints.push_back(2 * regionSize);

    // The last split point is at the bottom of the image
    splitPoints.push_back(IMAGE_HEIGHT);

    // Return the vector of split points.
    return splitPoints;
}

 
// Calculates the point with the highest density of white pixels in the
// region of the image bounded by the specified y-coordinates.
std::vector<int> findHighestDensityPoint(int y1, int y2)
{
    // The current highest density of white pixels
    int highestDensity = 0;

    // The coordinates of the point with the highest density of white pixels
    int highestDensityX = 0;
    int highestDensityY = 0;

    // Loop through the region of the image bounded by the specified y-coordinates
    for (int y = y1; y < y2; y++)
    {
        for (int x = 0; x < IMAGE_WIDTH; x++)
        {
            // Calculate the density of white pixels at the current point
            int density = 0;
            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    int xx = x + dx;
                    int yy = y + dy;
                    if (xx >= 0 && xx < IMAGE_WIDTH && yy >= 0 && yy < IMAGE_HEIGHT)
                    {
                        if (image[xx][yy] == 1)
                        {
                            density++;
                        }
                    }
                }
            }

            // Update the current highest density and coordinates if necessary
            if (density > highestDensity)
            {
                highestDensity = density;
                highestDensityX = x;
                highestDensityY = y;
                //Write vector of highest density point
                std::vector<int> highestDensityPoint = {highestDensityX, highestDensityY};
                return highestDensityPoint;
            }
        }
    }
}


//funtion determaning the vectors between the highest density points
std::vector<int> findVector(std::vector<int> highestDensityPoint1, std::vector<int> highestDensityPoint2)
{
    //The vector between the two highest density points
    std::vector<int> vector = {highestDensityPoint2[0] - highestDensityPoint1[0], highestDensityPoint2[1] - highestDensityPoint1[1]};
    return vector;
}





void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");
}

void loop() {
  // Check if there are incoming commands from the API
  if (WiFi.available()) {
    // Read the incoming command
    String command = WiFi.readStringUntil('\r');
    // Parse the JSON command
    DynamicJsonDocument jsonDoc(1024);
    DeserializationError error = deserializeJson(jsonDoc, command);
    if (error) {
      Serial.println("Error parsing JSON command: " + error.c_str());
    } else {
      // TODO: Add code here to handle the incoming command
    }
  }
}


