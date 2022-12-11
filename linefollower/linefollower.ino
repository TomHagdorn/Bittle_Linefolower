#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_camera.h>


#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems

//Depenencies for image processing
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>


// Pin definitions for the ESP32-CAM module
const int CAM_PIN_PWDN = -1;
const int CAM_PIN_RESET = -1;
const int CAM_PIN_XCLK = 0;
const int CAM_PIN_SIOD = 2;
const int CAM_PIN_SIOC = 15;
const int CAM_PIN_DATA = 4;

// Pin definition for the OV2640 camera module
const int OV2640_CAM_ADDRESS = 0x3C;

// Constants for the dimensions of the image
const int IMAGE_WIDTH = 640;
const int IMAGE_HEIGHT = 480;

// The image data, stored as a 2D array of pixels
int image[IMAGE_WIDTH][IMAGE_HEIGHT];

// Buffer to store the image data
uint8_t imageData[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

// Setup values for the program
// Region of the picture to be processed
int selectedRegion = 0;
// Threshold for the pixel density in a given area to be considered a line
int pxel_threshold = 100;

// Replace with your own Wi-Fi credentials
const char* ssid = "your_network_name";
const char* password = "your_network_password";

// Replace with your own API URL and port
const char* host = "your_api_url";
const int port = 80; // or whichever port your API is running on

// Functions

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

// Function that splits the image into 6 vertical regions and returns a vector
// containing the x-coordinates of the split points.
std::vector<int> splitImageVertically()
{
    // The size of each region is 1/6 of the image width
    int regionSize = IMAGE_WIDTH / 6;

    // The vector that will be returned, containing the x-coordinates
    // of the split points.
    std::vector<int> splitPoints;

    // The first split point is at the left of the image
    splitPoints.push_back(0);

    // The next five split points are at the middle of the first, second,
    // third, fourth, and fifth regions, respectively.
    splitPoints.push_back(regionSize);
    splitPoints.push_back(2 * regionSize);
    splitPoints.push_back(3 * regionSize);
    splitPoints.push_back(4 * regionSize);
    splitPoints.push_back(5 * regionSize);

    // The last split point is at the right of the image
    splitPoints.push_back(IMAGE_WIDTH);

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

// Function that counts the amount of white pixels in the region of the image
// bounded by the specified x and y-coordinates. Returns if the threshold for white pixels is met.
bool countWhitePixels(int x1, int x2, int y1, int y2)
{
    // The current amount of white pixels
    int whitePixels = 0;

    // Loop through the region of the image bounded by the specified x and y-coordinates
    for (int y = y1; y < y2; y++)
    {
        for (int x = x1; x < x2; x++)
        {
            // Count the white pixels at the current point
            if (image[x][y] == 1)
            {
                whitePixels++;
            }
        }
    }

    // Return if the threshold for white pixels is met
    if (whitePixels > pxel_threshold)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Function to modify the image for processing turns it black and white and inverts the colors
void modifyImage(esp_camera_fb_t* fb, int saturation, int contrast) {
  // Set the color filter to grayscale
  esp_err_t err = set_color_bar(0);
  if (err != ESP_OK) {
    Serial.println("Error setting color filter");
    return;
  }

  // Set the new saturation and contrast values
  err = set_saturation(saturation);
  if (err != ESP_OK) {
    Serial.println("Error setting saturation");
    return;
  }
  err = set_contrast(contrast);
  if (err != ESP_OK) {
    Serial.println("Error setting contrast");
    return;
  }

  // Set the image effect to invert
  err = set_image_effect(IMAGE_EFFECT_NEGATIVE);
  if (err != ESP_OK) {
    Serial.println("Error setting image effect");
    return;
  }

  // Capture a new image with the updated color filter and properties
  camera_fb_t* newFb = esp_camera_fb_get();
  if (!newFb) {
    Serial.println("Error capturing image");
    return;
  }

  // Copy the new image data to the original frame buffer
  memcpy(fb->buf, newFb->buf, newFb->len);
  fb->len = newFb->len;

  // Release the new frame buffer
  esp_camera_fb_return(newFb);
}

// function for thresholding the image
esp_camera_fb_t* thresholdImage(esp_camera_fb_t* fb, uint8_t threshold) {
  // Allocate a new frame buffer for the thresholded image
  camera_fb_t* thresholdedFb = (camera_fb_t*) calloc(1, sizeof(camera_fb_t));
  if (!thresholdedFb) {
    Serial.println("Error allocating frame buffer");
    return NULL;
  }

  // Threshold the image
  esp_err_t err = threshold_image(fb, threshold, thresholdedFb);
  if (err != ESP_OK) {
    Serial.println("Error thresholding image");
    return NULL;
  }

  // Return the thresholded image
  return thresholdedFb;
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

  // Initialize the ESP32-CAM module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = CAM_PIN_SIOD;
  config.pin_d1 = CAM_PIN_SIOC;
  config.pin_d2 = CAM_PIN_DATA;
  config.pin_d3 = CAM_PIN_DATA;
  config.pin_d4 = CAM_PIN_DATA;
  config.pin_d5 = CAM_PIN_DATA;
  config.pin_d6 = CAM_PIN_DATA;
  config.pin_d7 = CAM_PIN_DATA;
  config.pin_xclk = CAM_PIN_XCLK;
  // TODO add these funtions to node red flow
  // TODO Add a statemachine calling all the functions
  
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


  // Take a picture
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

}


