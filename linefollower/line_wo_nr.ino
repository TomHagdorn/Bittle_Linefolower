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

const char *ssid = "Get off my Lan!";
const char *password = "prettyflyforAWifi";

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

const framesize_t FRAME_SIZE_IMAGE = FRAMESIZE_SVGA; // FRAMESIZE_QQVGA
camera_fb_t *fb;

//! Image Format
/*!
    Other Available formats:
    YUV422, GRAYSCALE, RGB565, JPEG, RGB888
*/

#define PIXFORMAT PIXFORMAT_GRAYSCALE;



#define IMAGE_WIDTH 800  ///< Image size Width
#define IMAGE_HEIGHT 600 ///< Image size Height

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
int ledBrightness = 0;                     ///< Initial brightness (0 - 255)
const int pwmFrequency = 50000;            ///< PWM settings for ESP32
const uint8_t ledChannel = LEDC_CHANNEL_0; ///< Camera timer0
const uint8_t pwmResolution = 8;           ///< resolution (8 = from 0 to 255)

const int serialSpeed = 115200; ///< Serial data speed to use

// minimum line lenth for line detection
const int min_line_length = 500;

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
int pixel_threshold = 175;
int recover_time  = 500;
int Obst_left_t =1;
int Obst_straight_t=1;
int Obst_right_t =1;
int Line_width=1;
int Fin_line_width=1;
int currentlinewidth = 1;
int currentfinlinewidth =1;

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
        int start_time_loop = millis();

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
        lastCamera = millis();
        int runtime = lastCamera - start_time_loop;
        //Serial.println("Main loop runtime: " + String(runtime) + "ms");

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
    //int start_time_capture = millis();
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
            (*fb)->buf[index] = ((*fb)->buf[index] > pixel_threshold) ? 255 : 0;
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
    if (line_count >= 5)
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
    // Iterate over the rows in the image
    for (int row_index = 0; row_index < IMAGE_HEIGHT; ++row_index)
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

/**************************************************************************/
/**
  Line Follower
  Follow a line using the camera
  \param fb: pointer to the frame buffer
  \return true if a line is found, false otherwise
 */
/**************************************************************************/
bool line_follower(const camera_fb_t *fb)
{
    // calculate the starting and ending fractions for the 3/4 to 1 portion of the frame
    double start_fraction = 2.0 / 4.0;
    double end_fraction = 1.0;

    // get the middle point for the 3/4 to 1 portion of the frame
    int middle_point = get_middle_point(fb, start_fraction, end_fraction);
    if (middle_point == -1)
    {
        // no line was found, so stop the robot
        Serial.print("kbalance");
        return false;
    }

    // if the point of highest density is in one of the 3/7th of the left side of the picture
    if (middle_point < fb->width * 4 / 11)
    {
        // move the robot to the left
        Serial.print("kwkL");
        return true;
    }
    // if the point of highest density is in one of the 3/7th of the right side of the picture
    else if (middle_point >= fb->width * 8 / 11)
    {
        // move the robot to the right
        Serial.print("kwkR");
        return true;
    }
    // if the point of highest density is within the 4/7th in the middle
    else
    {
        // move the robot forward
        Serial.print("kwkF");
        return true;
    }
}

/**
 * @brief Finds the middle point of the white pixels in a region of the camera frame buffer.
 *
 * The function scans the specified region of the camera frame buffer for white pixels and calculates
 * the middle point of the white pixels. The middle point is determined by averaging the starting and
 * ending x-coordinates of the white pixels. The function also checks for the validity of the row
 * by checking for consecutive white pixels.
 *
 * @param fb The camera frame buffer.
 * @param start_fraction The fraction of the frame buffer's height at which to start scanning for white pixels.
 * @param end_fraction The fraction of the frame buffer's height at which to end scanning for white pixels.
 * @return The middle point of the white pixels in the specified region of the frame buffer, or -1 if no valid
 *         white pixels were found.
 */
int get_middle_point(const camera_fb_t *fb, double start_fraction, double end_fraction)
{
    // initialize the starting and ending x-coordinates to zero
    int start_x = 0;
    int end_x = 0;
    // flag to track if we have found the start of the white pixels
    bool found_start = false;
    // variable to track the number of consecutive non-white pixels
    int consecutive_non_white = 0;
    // variable to track the number of consecutive white pixels to check row validity
    int consecutive_white = 0;
    // row counter
    int valid_row_counter = 0;
    // vector to store the middle points of each row
    std::vector<int> middle_points;

    // calculate the start and end rows based on the start and end fractions
    int start_row = fb->height * start_fraction;
    int end_row = fb->height * end_fraction;
    // check if the start and end rows are valid
    if ((end_row > fb->height) || (start_row < 0) || (start_row > end_row))
    {
        // invalid start and end rows, so return -1
        Serial.println("Invalid start and end rows");
        Serial.println("Height: " + String(fb->height) + ", Start Row: " + String(start_row) + ", End Row: " + String(end_row));
        Serial.println("Need 0 <= start_row < end_row <= height");
        return -1;
    }
    // iterate over the rows in the specified range
    for (int y = start_row; y < end_row; y++)
    {
        // iterate over the columns in the current row
        for (int x = 0; x < fb->width; x++)
        {
            // get the current pixel value
            uint8_t pixel = fb->buf[y * fb->width + x];

            // if the pixel is white (i.e., its value is above the threshold)
            if (pixel == 255)
            {
                // if we haven't found the start of the white pixels yet,
                // set the start x-coordinate to the current x-coordinate
                consecutive_white++;
                // reset the consecutive non-white pixels counter
                consecutive_non_white = 0;
                // if we haven't found the start of the white pixels yet,
                if (!found_start)
                {
                    start_x = x;
                    found_start = true;
                }
                // update the ending x-coordinate to the current x-coordinate
                end_x = x;
            }
            else
            {
                // increment the consecutive non-white pixels counter
                consecutive_non_white++;
            }
            // if we have seen 15 consecutive non-white pixels, set the end x-coordinate to
            // the current x-coordinate minus 15
            if (consecutive_non_white >= 40)
            {
                end_x = x - 40;
                consecutive_non_white = 0;
                found_start = false;
            }
            // if we have seen 40 consecutive white pixels, then we have found a valid row
            if (consecutive_white >= 100 && found_start)
            {
                valid_row_counter++;
                // calculate the middle x-coordinate of the white pixels
                int middle_point = (end_x + start_x) / 2;
                // add the middle point to the vector
                middle_points.push_back(middle_point);
            }
        }
    }
    // if we have found at least one valid row
    if (valid_row_counter > 30)
    {
        // sort the middle points
        std::sort(middle_points.begin(), middle_points.end());
        // calculate the median middle point
        int median_middle_point = middle_points[middle_points.size() / 2];
        // return the median middle point
        return median_middle_point;
    }
    // if we haven't found any valid rows, return -1
    else
    {
        return -1;
    }
}

/**************************************************************************/
/**
  Detect Obstacle
  Detect an obstacle using the camera
  \param fb: pointer to the frame buffer
  \return true if an obstacle is found, false otherwise
 */
bool detect_obstacle(const camera_fb_t *fb)
{
    // calculate the starting and ending fractions for the top, middle, and bottom thirds of the frame
    // double start_fraction_top = 0.0;
    // double end_fraction_top = 1.0 / 3.0;
    // double start_fraction_middle = 1.0 / 3.0;
    // double end_fraction_middle = 2.0 / 3.0;
    // double start_fraction_bottom = 2.0 / 3.0;
    // double end_fraction_bottom = 1.0;

    // // get the middle points for the top, middle, and bottom thirds of the frame
    // int middle_point_top = get_middle_point(fb, start_fraction_top, end_fraction_top);
    // int middle_point_middle = get_middle_point(fb, start_fraction_middle, end_fraction_middle);
    // int middle_point_bottom = get_middle_point(fb, start_fraction_bottom, end_fraction_bottom);

    // // if the middle point for the bottom third of the frame is not -1 (i.e., a valid row was found),
    // // and the middle points for the top and middle thirds are -1 (i.e., no valid rows were found),
    // // then we consider an obstacle to have been detected
    // if (middle_point_bottom != -1 && middle_point_top == -1 && middle_point_middle == -1)
    // {
    //     return true;
    // }
    // else
    // {
    //     return false;
    // }
    return false;
}


void avoid_obstacle() {
    //TODO fix this function
    // // Stop the robot
    // Serial.print("kp");
    // // Turn the robot left for a certain amount of time
    // unsigned long start_time = millis();
    // Serial.print("kwkL");
    // while (millis() - start_time < 500) {
    //     // Wait
    
    // }

    // // Turn the robot right until the line is found or until a timeout occurs
    // start_time = millis();
    // Serial.print("kwkR");
    // int middle_point;
    // while ((middle_point = get_middle_point(fb, 1.0/2.0, 2.0/3.0)) == -1) {
    //     // Keep checking the middle point until it's not -1
    //     if(millis() - start_time >= 5000) {
    //         Serial.println("Error: Timed out while trying to find line.");
    //         return;
    //     }
    // }
    // // Once the line is found, turn the robot left for a certain amount of time
    // start_time = millis();
    // Serial.print("kwkL");
    // while (millis() - start_time < 200) {
    //     // Wait
    // }
    // // Stop the robot
    // Serial.print("kp");
    // return;
    return;
}

void recover()
{
    unsigned long start_time = millis();
    // walk backwards for a certain amount of time
    Serial.print("kwkB");
    while(millis() - start_time < 1500) {
    }
    Serial.print("kbalance");
    return;
}

void cool_move()
{
    // TODO David writes cool move and then we test
    //write function to turn on the spot
    return;
}

void crossFinishLine()
{
    // walk backwards for a certain amount of time
    Serial.print("kwkF");
    unsigned long start_time = millis();
    while (millis() - start_time < 400)
    {
    }
    Serial.print("kbalance");
    return;
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
            if (detect_obstacle(fb) && millis() - lastStateChangeTime >= 4000)
            {
                currentState = AVOID_OBSTACLE;
                lastStateChangeTime = millis();
            }
            else if (check_for_horizontal_line(fb) && (millis() - startTime >= 60000))
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


