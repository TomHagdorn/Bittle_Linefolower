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
// Depenencies for image processing

#include <WiFi.h>
#include <WebServer.h>

#include <cmath>

//My files 
#include "control.h"

//const char *ssid = "Get off my Lan!";
//const char *password = "prettyflyforAWifi";

unsigned long stateTime = 0;

WebServer server(80);

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

// variables updated by nodered
int pixel_threshold = 200;
int recover_time  = 500;
int Obst_left_t =1;
int Obst_straight_t=1;
int Obst_right_t =1;
int Line_width=1;
int Fin_line_width=1;
int currentlinewidth = 1;
int currentfinlinewidth =1;
int obstacle_detection_dist = 15;

//filter settings
const int kernelSize = 3; // kernel size for gaussian blur

// poiner to the sobel gradient
//int *gradient;

// Pin definitions for ultrasound sensor
const int trigPin = 12;
const int echoPin = 13;

//Pin definitions for LEDs
const int led_control = 4;

//possible states of the robot
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
    setLedBrightness(ledBrightness);
    // Ultrasound sensor setup
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input
    //Nodered setup

    

}
/**************************************************************************/
/*!
  \brief  Loop function
  Capture image every 10 seconds
*/
/**************************************************************************/
void loop()
{       
    
    if ((unsigned long)(millis() - lastCamera) >=700UL)
    {   
        
        esp_err_t res = camera_capture(&fb);
        if (res == ESP_OK)
        {   
            //Serial.println(pixel_threshold);
            update();
            
            // print image to serial monitor
            //capture_still(fb);

            // publishes the image to the server
            // publishPictureToServer(fb, server);
        }
        // return the frame buffer back to the driver for reuse
        esp_camera_fb_return(fb);
        //free(gradient);
        // free the gradient
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

    // int height = (*fb)->height;
    // int width = (*fb)->width;

    // int *gradient = (int*) malloc(height * width * sizeof(int));

    // gaussianBlur(*fb, kernelSize);

    // sobel(*fb, gradient);

    // threshold(*fb, pixel_threshold, gradient);

    // free(gradient);
    threshold_image(*fb, pixel_threshold);

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
 * Update the current state of the line follower.
 *
 * This function updates the current state of the line follower based on the
 * current conditions of the line, the presence of obstacles, and the presence
 * of the finish line. It then executes the appropriate behavior based on the
 * current state.
 *
 * The possible states are:
 * - FOLLOW_LINE: Follow the line using the camera.
 * - AVOID_OBSTACLE: Avoid an obstacle by walking around it.
 * - CROSS_FINISH_LINE: Cross the finish line.
 * - FINISH: The robot has finished the course.
 * - RECOVER_FROM_NO_LINE: Recover from losing the line by walking backwards in a curve.
 *
 * \param fb: pointer to the frame buffer.
 */
/**************************************************************************/
void update()
{
    static unsigned long startTime = millis();

    switch (currentState)
    {
    case FOLLOW_LINE:
        //Serial.println("\nFOLLOW_LINE");
        if (line_follower(fb))
        {
            lastStateChangeTime = millis();
            // line follower returned true, indicating that the line was found
            if (detect_obstacle())
            {
                currentState = AVOID_OBSTACLE;
                lastStateChangeTime = millis();
            }
            else if (check_for_horizontal_line(fb,min_line_length) && (millis() - startTime >= 60000))
            {
                currentState = CROSS_FINISH_LINE;
                lastStateChangeTime = millis();
            }
        }
        else
        {
            if (millis() - lastStateChangeTime >= 9000)
            {
                // line follower returned false, indicating that the line was not found
                // and at least 3 seconds have passed since the last state change
                currentState = RECOVER_FROM_NO_LINE;
                lastStateChangeTime = millis();
            }
        }
        break;
    case AVOID_OBSTACLE:
        //Serial.println("\nAVOID_OBSTACLE");
        avoid_obstacle();
        currentState = FOLLOW_LINE;
        lastStateChangeTime = millis();

        break;
    case CROSS_FINISH_LINE:
        //Serial.println("\nCROSS_FINISH_LINE");
        crossFinishLine();
        if (finish_line_crossed == false)
        {
            finish_line_crossed = true;
            cool_move();
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
        }
        else
        {
            currentState = FINISH;
        }
        break;
    case RECOVER_FROM_NO_LINE:
        //Serial.println("\nRECOVER_FROM_NO_LINE");
        recover();
        // line follower returned true, indicating that the line was found
        currentState = FOLLOW_LINE;
        lastStateChangeTime = millis();
        break;
    case FINISH:
        // Robot has finished, do nothing
        break;
    }
}
