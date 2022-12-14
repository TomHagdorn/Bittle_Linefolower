/*!
    \file camera_dump.ino

    \brief Dump array captured from camera to serial.

    The image captured is stored in a form of 1D array. This image is a grayscale image.
    The contrast and exposure is set automatically.

    \author Tom Hagdorn 
*/

/*!
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



//! Image resolution:
/*!
    default = "const framesize_t FRAME_SIZE_IMAGE = FRAMESIZE_VGA"

    Other available Frame Sizes:
    160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA),
    320x240 (QVGA), 400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA),
    1024x768 (XGA), 1280x1024 (SXGA), 1600x1200 (UXGA)
*/

const framesize_t FRAME_SIZE_IMAGE = FRAMESIZE_QQVGA;   //FRAMESIZE_QQVGA
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
int ledBrightness = 16;                    ///< Initial brightness (0 - 255)
const int pwmFrequency = 50000;            ///< PWM settings for ESP32
const uint8_t ledChannel = LEDC_CHANNEL_0; ///< Camera timer0
const uint8_t pwmResolution = 8;           ///< resolution (8 = from 0 to 255)

const int serialSpeed = 115200; ///< Serial data speed to use

//minimum line lenth for line detection
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
    Serial.println("\n");

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); ///< Disable 'brownout detector'

    Serial.print(("\nInitialising camera: "));
    if (initialiseCamera())
    {
        Serial.println("OK");
    }
    else
    {
        Serial.println("Error!");
    }

    setupOnBoardFlash();
    Serial.println("Setup complete\n");
    setLedBrightness(ledBrightness);
}
/**************************************************************************/
/*!
  \brief  Loop function
  Capture image every 10 seconds
*/
/**************************************************************************/
void loop()
{


    
    if ((unsigned long)(millis() - lastCamera) >= 1000UL)
    {
        esp_err_t res = camera_capture(&fb);
            if (res == ESP_OK) {
                // use the frame buffer here

            //capture_still(fb);
            lastCamera = millis(); // reset timer

            if (linefollower(fb) == true)  
            {
                //move the robot forward
                //TODO add code to move the robot forward
                Serial.println("Finsh line detected");

            }
            else
            {
                //stop the robot
            }         // function call -> capture image
            //return the frame buffer back to the driver for reuse
            esp_camera_fb_return(fb);
        }
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
        Serial.println("Warning: No PSRam found so defaulting to image size 'CIF'");
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
        Serial.println("Error: problem reading camera sensor settings");
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
//TODO try if this works
esp_err_t camera_capture(camera_fb_t **fb) {
    // acquire a frame
    *fb = esp_camera_fb_get();
    ESP_LOGE(TAG, "Camera Capture in progress");
    if (!*fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }

    // get the height and width of the frame
    int height = (*fb)->height;
    int width = (*fb)->width;

    // calculate the starting and ending row indices for the lowest third of the frame
    int startRow = 2 * height / 3;
    int endRow = height - 1;

    // threshold the lowest third of the frame
    for (int row = startRow; row <= endRow; row++) {
        for (int col = 0; col < width; col++) {
            int index = row * width + col;

            // threshold the pixel at the current index
            (*fb)->buf[index] = ((*fb)->buf[index] < 150) ? 255 : 0;
        }
    }
    // print serial ok
    Serial.println("Camera Capture OK");

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
  Get Middle Point
  Get the middle point of the white pixels in the bottom third of the image
  \param fb: pointer to the frame buffer
  \return the x-coordinate of the middle point
 */
/**************************************************************************/
int get_middle_point(const camera_fb_t *fb)
{
    // initialize the starting and ending x-coordinates to zero
    int start_x = 0;
    int end_x = 0;
    int median_end_x = 0;
    int median_start_x = 0;
    // flag to track if we have found the start of the white pixels
    bool found_start = false;
    // variable to track the number of consecutive non-white pixels
    int consecutive_non_white = 0;
    // variable to track the number of consecutive white pixels to check row validity
    int consecutive_white = 0;
    // row counter
    int valid_row_counter = 0;
    

    // iterate over the rows in the bottom fourth of the image
    for (int y = fb->height * 3/4; y < fb->height; y++) {
        // iterate over the columns in the current row
        for (int x = 0; x < fb->width; x++) {
            // get the current pixel value
            uint8_t pixel = fb->buf[y * fb->width + x];

            // if the pixel is white (i.e., its value is above the threshold)
            if (pixel == 255) {
                // if we haven't found the start of the white pixels yet,
                // set the start x-coordinate to the current x-coordinate
                consecutive_white++;
                if (!found_start) {
                    start_x = x;
                    found_start = true;
                }
                // reset the consecutive non-white pixels counter
                consecutive_non_white = 0;
                // update the ending x-coordinate to the current x-coordinate
                end_x = x;
            } else {
                // increment the consecutive non-white pixels counter
                consecutive_non_white++;
            }
            // if we have seen 15 consecutive non-white pixels, set the end x-coordinate to
            // the current x-coordinate minus 15
            if (consecutive_non_white >= 15) {
            end_x =  x - 15;
            break;
            }
        }
        // if we have seen 40 consecutive white pixels, then we have found a valid row
        if (consecutive_white >= 40) {
            valid_row_counter++;
            median_end_x = median_end_x + end_x;
            median_start_x = median_start_x + start_x;
        }

    // calculate the middle x-coordinate of the white pixels
    int x_middle_point = (median_end_x + median_start_x) / (2 * valid_row_counter);
    // return the middle x-coordinate
    Serial.println(x_middle_point);
    return x_middle_point;
    }
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

    // iterate over the rows in the lowest third of the image
    for (int y = start_row; y <= end_row; y++) {
        // initialize the consecutive white pixel count to zero
        int white_pixel_count = 0;

        // iterate over the pixels in the current row
        for (int x = 0; x < fb->width; x++) {
            // get the current pixel value
            uint8_t pixel = fb->buf[y * fb->width + x];

            // if the pixel is white (i.e., its value is above the threshold)
            // then increment the consecutive white pixel count
            if (pixel == 255) {
                white_pixel_count++;
            }
            // if the pixel is not white (i.e., its value is below the threshold)
            // then reset the consecutive white pixel count
            else {
                white_pixel_count = 0;
            }

            // if there are at least min_line_length consecutive white pixels
            // then return true (i.e., a horizontal line has been found)
            if (white_pixel_count >= min_line_length) {
                //print serial ok
                Serial.println("Horizontal Line Found");
                return true;
            }
        }
    }

    // if a horizontal line was not found, then return false
    return false;
}

/**************************************************************************/
/**
  Line Follower
  Follow a line using the camera
  \param fb: pointer to the frame buffer
  \return true if a line is found, false otherwise
 */
/**************************************************************************/
bool linefollower(const camera_fb_t *fb)
{

    // get the point of highest density in the image
    int middle_point = get_middle_point(fb);
    if (middle_point == 0) {
        //print serial ok
        Serial.println("No Line Found");
        return false;
    }

    // if the point of highest density is in one of the 3/7th of the left side of the picture
    if (middle_point < fb->width * 3 / 7 ) {
        // move the robot to the left
        //print move left
        Serial.println("Robot moves left");
    }
    // if the point of highest density is in one of the 3/7th of the right side of the picture
    else if (middle_point >= fb->width * 4 / 7) {
        // move the robot to the right
        //print move right
        Serial.println("Robot moves right");
    }
    // if the point of highest density is within the 4/7th in the middle
    else {
        // move the robot forward
        //print move forward
        Serial.println("Robot moves forward");
    }

    // check if a horizontal line was detected
    // if (check_for_horizontal_line(fb)) {
    //     // if a horizontal line was detected, then walk forward for 1 second
    //     return true;
    // }
    // else {
    // // if no horizontal line was detected, then return false
    //return false;
    //}
    return false;
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

    for (int row_index = 0; row_index < IMAGE_HEIGHT; ++row_index)
    {
        for (int col_index = 0; col_index < IMAGE_WIDTH; ++col_index)
        {
            Serial.print(fb->buf[IMAGE_WIDTH * row_index + col_index]);
            Serial.print(" ");
        }
        Serial.print("\n");
    }
    Serial.println();
    return true;
}





   