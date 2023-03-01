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


#include "soc/soc.h"          //! Used to disable brownout detection
#include "soc/rtc_cntl_reg.h" //! Used to disable brownout detection

#include <vector>


//My files 
#include "camera_setup.h"
#include "control.h"
#include "light_strip.h"
#include "node_red.h"
#include "calibration.h"





unsigned long stateTime = 0;

uint32_t lastCamera = 0; ///< To store time value for repeated capture

//define state change time
static unsigned long lastStateChangeTime = 0;

// variables updated by nodered
//int pixel_threshold = 125;
int recover_time  = 500;
int Obst_left_t =1;
int Obst_straight_t=1;
int Obst_right_t =1;
int Line_width=1;
int Fin_line_width=1;
int currentlinewidth = 1;
int currentfinlinewidth =1;
int obstacle_detection_dist = 15;
// minimum line lenth for line detection
const int min_line_length = 10;

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
    //Node red setup TODO Needs to be moved to a seperate file in a function
    Change_Treshold_value();

    //captureAndSendImage();
    // Ultrasound sensor setup
    strip_setup();
    
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input

    

}
/**************************************************************************/
/*!
  \brief  Loop function
  Capture image every 10 seconds
*/
/**************************************************************************/
void loop()
{    
    cycle_led_strip();
    
    if ((unsigned long)(millis() - lastCamera) >=700UL)
    {   
        

        esp_err_t res = camera_capture();
        // bool res = true;
        
        // if (res = true)
        if (res == ESP_OK)
        {   
            //Serial.println(pixel_threshold);
            update();
            //update_server();
            // print image to serial monitor
            //threshold_image(fb,pixel_threshold);
            //capture_still(fb);
            

        }


        //return the frame buffer back to the driver for reuse
        esp_camera_fb_return(fb);
        
        //free(gradient);
        // free the gradient
        lastCamera = millis();
    }
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

