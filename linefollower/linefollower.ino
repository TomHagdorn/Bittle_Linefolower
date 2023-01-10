/**
 * @file linefollower.ino
 * @brief Line follower robot using a camera.
 *
 * This file contains the code for a line follower robot that uses a camera to
 * detect and follow a line. The robot has several behaviors, including
 * following the line, avoiding obstacles, crossing the finish line, and
 * recovering from losing the line.

    \author Tom Hagdorn
*/

/**
    \brief Check for ESP32 board.
*/

#if !defined ESP32
#error Wrong board selected
#endif

#define CAMERA_MODEL_AI_THINKER

#include "esp_camera.h" ///< Header file for camera obtained from https://github.com/espressif/

#include "driver/ledc.h" ///< To enable onboard Illumination/flash LED pin attached on 4

#include "soc/soc.h"          //! Used to disable brownout detection
#include "soc/rtc_cntl_reg.h" //! Used to disable brownout detection

#include <vector> 
//Depenencies for image processing

#include <WiFi.h>

const char *ssid = "Get off my Lan!";
const char *password = "prettyflyforAWifi";

unsigned long stateTime = 0;

//! Image resolution:
/*!
    default = "const framesize_t FRAME_SIZE_IMAGE = FRAMESIZE_VGA"

    Other available Frame Sizes:
    160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA),
    320x240 (QVGA), 400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA),
    1024x768 (XGA), 1280x1024 (SXGA), 1600x1200 (UXGA)
*/

const framesize_t FRAME_SIZE_IMAGE = FRAMESIZE_QQVGA; // FRAMESIZE_QQVGA
camera_fb_t *fb;

//! Image Format
/*!
    Other Available formats:
    YUV422, GRAYSCALE, RGB565, JPEG, RGB888
*/

#define PIXFORMAT PIXFORMAT_GRAYSCALE;

int pixel_threshold = 30;

#define IMAGE_WIDTH 160  ///< Image size Width
#define IMAGE_HEIGHT 120 ///< Image size Height

//! Camera exposure
/*!
    Range: (0 - 1200)
    If gain and exposure both set to zero then auto adjust is enabled
*/

int cameraImageExposure = 0;

//! Image gain
/*!
    Range: (0 - 30)
    If gain and exposure both set to zero then auto adjust is enabled
*/
int cameraImageGain = 0;

const uint8_t ledPin = 4;                  ///< onboard Illumination/flash LED pin (4)
int ledBrightness = 8;                     ///< Initial brightness (0 - 255)
const int pwmFrequency = 50000;            ///< PWM settings for ESP32
const uint8_t ledChannel = LEDC_CHANNEL_0; ///< Camera timer0
const uint8_t pwmResolution = 8;           ///< resolution (8 = from 0 to 255)

const int serialSpeed = 115200; ///< Serial data speed to use

// minimum line lenth for line detection
const int min_line_length = 10;

//! Camera setting
/*!
    Camera settings for CAMERA_MODEL_AI_THINKER OV2640
    Based on CameraWebServer sample code by ESP32 Arduino

*/
#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#endif

uint32_t lastCamera = 0; ///< To store time value for repeated capture

enum State
{
    FOLLOW_LINE,
    AVOID_OBSTACLE,
    CROSS_FINISH_LINE,
    FINISH,
    RECOVER_FROM_NO_LINE,
};

State currentState = FOLLOW_LINE;
// bool to decide if the finish line has been crossed already
bool finish_line_crossed = false;
// define the wifi server
WiFiServer server(80);

//define state change time
static unsigned long lastStateChangeTime = 0;
/**************************************************************************/
/*!
  \brief  Setup function

  Initialization for following:
    disable Brownout detection
    camera

*/
/**************************************************************************/
void setup()
{
    Serial.begin(serialSpeed); ///< Initialize serial communication

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); ///< Disable 'brownout detector'

    // Serial.print(("\nInitialising camera: "));
    if (initialiseCamera())
    {
        // Serial.println("OK");
    }
    else
    {
        // Serial.println("Error!");
    }

    setupOnBoardFlash();
    // Serial.println("Setup complete\n");
    setLedBrightness(ledBrightness);
    // Start the web server for image upload comment out for normal operation
    //  setupWebServer(server);
    //  Serial.println(WiFi.begin(ssid, password));
}
/**************************************************************************/
/*!
  \brief  Loop function
  Capture image every 10 seconds
*/
/**************************************************************************/
void loop()
{
    if ((unsigned long)(millis() - lastCamera) >= 600UL)
    {
        esp_err_t res = camera_capture(&fb);
        if (res == ESP_OK)
        {
            update();

            // print image to serial monitor
            // capture_still(fb);

            // publishes the image to the server
            // publishPictureToServer(fb, server);
        }
        // return the frame buffer back to the driver for reuse
        esp_camera_fb_return(fb);
        lastCamera = millis();
    }
}
/**************************************************************************/
/**
  Initialise Camera
  Set camera parameters
  Based on CameraWebServer sample code by ESP32 Arduino
  \return true: successful, false: failed
 */
/**************************************************************************/
bool initialiseCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT;
    config.frame_size = FRAME_SIZE_IMAGE;
    config.jpeg_quality = 10;
    config.fb_count = 1;

    // Check the esp32cam board has a PSRAM chip installed (extra memory used for storing captured images)
    // Note: if not using "AI thinker esp32 cam" in the Arduino IDE, PSRAM must be enabled
    if (!psramFound())
    {
        // Serial.println("Warning: No PSRam found so defaulting to image size 'CIF'");
        config.frame_size = FRAMESIZE_CIF;
    }

    esp_err_t camerr = esp_camera_init(&config); // initialise the camera
    if (camerr != ESP_OK)
    {
        Serial.printf("ERROR: Camera init failed with error 0x%x", camerr);
    }

    cameraImageSettings(); // Apply custom camera settings

    return (camerr == ESP_OK); // Return boolean result of camera initialisation
}
/**************************************************************************/
/**
  Camera Image Settings
  Set Image parameters
  Based on CameraWebServer sample code by ESP32 Arduino
  \return true: successful, false: failed
 */
/**************************************************************************/
bool cameraImageSettings()
{

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL)
    {
        // Serial.println("Error: problem reading camera sensor settings");
        return 0;
    }

    // if both set to zero enable auto adjust
    if (cameraImageExposure == 0 && cameraImageGain == 0)
    {
        // enable auto adjust
        s->set_gain_ctrl(s, 1);     // auto gain on
        s->set_exposure_ctrl(s, 1); // auto exposure on
        s->set_awb_gain(s, 1);      // Auto White Balance enable (0 or 1)
    }
    else
    {
        // Apply manual settings
        s->set_gain_ctrl(s, 0);                   // auto gain off
        s->set_awb_gain(s, 1);                    // Auto White Balance enable (0 or 1)
        s->set_exposure_ctrl(s, 0);               // auto exposure off
        s->set_agc_gain(s, cameraImageGain);      // set gain manually (0 - 30)
        s->set_aec_value(s, cameraImageExposure); // set exposure manually  (0-1200)
    }

    return true;
}
/**************************************************************************/
/**
  Camera Image Settings
  Set Image parameters
  Based on CameraWebServer sample code by ESP32 Arduino
  \return true: successful, false: failed
 */
/**************************************************************************/
esp_err_t camera_capture(camera_fb_t **fb)
{
    // acquire a frame
    *fb = esp_camera_fb_get();
    ESP_LOGE(TAG, "Camera Capture in progress");
    if (!*fb)
    {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }

    // get the height and width of the frame
    int height = (*fb)->height;
    int width = (*fb)->width;

    // threshold the entire frame
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int index = row * width + col;

            // threshold the pixel at the current index
            // if the pixel is less than 210, set it to 255 (white)
            (*fb)->buf[index] = ((*fb)->buf[index] < 25) ? 255 : 0;
        }
    }
    // print serial ok
    // Serial.println("Camera Capture OK");

    return ESP_OK;
}
/**************************************************************************/
/**
  Setup On Board Flash
  Initialize on board LED with pwm channel
 */
/**************************************************************************/

void setupOnBoardFlash()
{
    ledcSetup(ledChannel, pwmFrequency, pwmResolution);
    ledcAttachPin(ledPin, ledChannel);
}
/**************************************************************************/
/**
  Set Led Brightness
  Set pwm value to change brightness of LED
 */
/**************************************************************************/
void setLedBrightness(byte ledBrightness)
{
    ledcWrite(ledChannel, ledBrightness);
}

/**************************************************************************/
/**
  Check for Horizontal Line
  Check if there is a horizontal line in the bottom third of the image
  \param fb: pointer to the frame buffer
  \return true if there is a horizontal line, false otherwise
 */
/**************************************************************************/
bool check_for_horizontal_line(const camera_fb_t *fb)
{
    // calculate the start and end rows of the lowest third of the image
    int start_row = fb->height * 2 / 3;
    int end_row = fb->height - 1;

    // counter for the number of lines found
    int line_count = 0;

    // iterate over the rows in the lowest third of the image
    for (int y = start_row; y <= end_row; y++)
    {
        // initialize the consecutive white pixel count to zero
        int white_pixel_count = 0;

        // iterate over the pixels in the current row
        for (int x = 0; x < fb->width; x++)
        {
            // get the current pixel value
            uint8_t pixel = fb->buf[y * fb->width + x];

            // if the pixel is white (i.e., its value is above the threshold)
            // then increment the consecutive white pixel count
            if (pixel == 255)
            {
                white_pixel_count++;
            }
            // if the pixel is not white (i.e., its value is below the threshold)
            // then reset the consecutive white pixel count
            else
            {
                white_pixel_count = 0;
            }

            // if there are at least min_line_length consecutive white pixels
            // then increment the line count
            if (white_pixel_count >= min_line_length)
            {
                line_count++;
                break;
            }
        }
    }

    // if three or more lines were found, return true
    if (line_count >= 3)
    {
        return true;
    }
    // otherwise, return false
    else
    {
        return false;
    }
}

/**************************************************************************/
/**
  Capture Still
  Capture a still image and print the pixel values to the serial monitor
  \param fb: pointer to the frame buffer
  \return true if the image was captured successfully, false otherwise
 */
/**************************************************************************/
bool capture_still(const camera_fb_t *fb)
{
    // Calculate the height of the lowest third of the image
    int third_height = IMAGE_HEIGHT / 3;

    // Iterate over the rows in the lowest third of the image
    for (int row_index = 2 * third_height; row_index < IMAGE_HEIGHT; ++row_index)
    {
        // Iterate over the columns in the current row
        for (int col_index = 0; col_index < IMAGE_WIDTH; ++col_index)
        {
            // Print the value of the current pixel
            Serial.print(fb->buf[IMAGE_WIDTH * row_index + col_index]);
            Serial.print(" ");
        }
        // Move to the next line after printing the values for the current row
        Serial.print("\n");
    }
    Serial.println();
    // Return true to indicate that the still was successfully captured
    return true;
}

/**
 * @brief Automatically sets the threshold for a camera frame buffer so that the number of white pixels in the thresholded image falls within a given range.
 *
 * @param fb Pointer to a camera frame buffer struct.
 * @param min_white_pixels Minimum acceptable number of white pixels in the thresholded image.
 * @param max_white_pixels Maximum acceptable number of white pixels in the thresholded image.
 */
int auto_threshold(camera_fb_t *fb, int min_white_pixels, int max_white_pixels)
{
    // Set up variables for thresholding
    uint8_t *data = fb->buf;
    int width = fb->width;
    int height = fb->height;
    int threshold = 0;
    int white_pixels = 0;

    // Loop through threshold values until the number of white pixels is within the desired range
    while (white_pixels < min_white_pixels || white_pixels > max_white_pixels)
    {
        white_pixels = 0;
        for (int i = 0; i < width * height; i++)
        {
            // If the pixel value is greater than the threshold, consider it white
            if (data[i] > threshold)
            {
                white_pixels++;
            }
        }

        // Adjust the threshold for the next iteration
        if (white_pixels < min_white_pixels)
        {
            threshold++;
        }
        else if (white_pixels > max_white_pixels)
        {
            threshold--;
        }
    }

  // Once the desired range has been reached, apply the threshold to the image
  for (int i = 0; i < width * height; i++) {
    if (data[i] > threshold) {
      data[i] = 255;
    } else {
      data[i] = 0;
    }
  }
}


