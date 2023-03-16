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
#define BAUDRATE 115200

#include "soc/soc.h"          //! Used to disable brownout detection
#include "soc/rtc_cntl_reg.h" //! Used to disable brownout detection

#include <vector>

const int serialSpeed = 115200; ///< Serial data speed to use
// My files
#include "nodeRed_variables.h"
#include "camera_setup.h"
#include "testing.h"
#include "control.h"
#include "light_strip.h"
#include "node_red.h"
#define PIN_S2TX 4

// possible states of the robot
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

unsigned long lastServerUpdate = 0;
unsigned long startTime = 0;
unsigned long obstacleTime = 7000;



/**
 * Initializes various components used in the device.
 * Initializes serial communication, disables brownout detector, initializes the camera,
 * sets up the on-board flash, sets LED brightness, and sets up the ultrasound sensor.
 * Additionally, it sets the modes of the trigPin and echoPin.
 *
 * @return None.
 */
void setup()
{
    Serial.begin(serialSpeed); ///< Initialize serial communication
    BittleSerial.begin(BAUDRATE ,SERIAL_8N1, PIN_S2TX);
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
    setup.wifi();
    setupOnBoardFlash();
    setLedBrightness(ledBrightness);
    server.on("/Start", HandleClienttrue);
    server.on("/Stop", HandleClientfalse);
    // Wifi functions to start or stop the update()
    server.on("/status", handle_status);
 
    send_image();
    Update_node_red_value();


    // Ultrasound sensor setup
    strip_setup();

    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
}
/**
 * The main loop of the program.
 *
 * Performs image processing on the camera capture every 320 milliseconds, including applying
 * a Gaussian blur, thresholding, and updating the device's state accordingly. Also updates
 * the server every 900 milliseconds (commented out). Prints the loop time to the serial monitor.
 *
 * @return None.
 */
void loop()
{

    //cycle_led_strip();

    if (millis() - lastCamera > 320)
    {
        // unsigned long loopTimeStart = millis();
        esp_err_t res = camera_capture();

        if (res == ESP_OK)
        {
            gaussianBlur(kernelSize);
            // sobel();
            threshold_image();
            unsigned long thresholdTime = millis();
            //  Serial.print("Threshold time: ");
            //  Serial.println(thresholdTime - loopTimeStart);
            update();
            unsigned long updateTime = millis();
            // Serial.print("Update time: ");
            // Serial.println(updateTime - thresholdTime);
            update_movement();
            // unsigned long movementTime = millis();
            //  Serial.print("Movement time: ");
            //  Serial.println(movementTime - updateTime);

            // update server every 600ms to save resources
            // TODO Fix this
            if (millis() - lastServerUpdate >= 900)
            {
                // server.handleClient();
                // lastServerUpdate = millis();
            }

            // print image to serial monitor
            // capture_still();
            // free the sobel image gradient buffer
            // delete[] gradient;
            lastCamera = millis();
        }
        // unsigned long loopTimeEnd = millis();
        // Serial.print("Loop time: ");
        // Serial.println(loopTimeEnd - loopTimeStart);
        // return the frame buffer back to the driver for reuse
        esp_camera_fb_return(fb);
    }
    // Print the loop timeg
}

/**
 * The `update()` function updates the state of the robot based on the current state.
 * This function is called repeatedly in the `loop()` function.
 * 
 * The robot has several states, including:
 * 
 * - FOLLOW_LINE: the robot follows the line on the ground.
 * - AVOID_OBSTACLE: the robot detects an obstacle and maneuvers around it.
 * - CROSS_FINISH_LINE: the robot crosses the finish line.
 * - RECOVER_FROM_NO_LINE: the robot has lost the line and tries to recover.
 * - FINISH: the robot has finished the course.
 * 
 * Depending on the current state, this function may call other functions to perform specific actions,
 * such as line following, obstacle avoidance, or finish line detection.
 * 
 * This function also sets the current movement state of the robot, which is used to control the
 * motors and move the robot.
 */
void update()
{

    switch (currentState)
    {
    case FOLLOW_LINE:
        if (detect_obstacle() == true && (millis() - obstacleTime >= 5000))
        {
            //Serial.println("\nAVOID_OBSTACLE");
            currentState = AVOID_OBSTACLE;
            lastStateChangeTime = millis();
        }
        else
        {

            if (check_for_horizontal_line() && (millis() - finishTime >= 3000))
            {
                //Serial.println("\nCROSS_FINISH_LINE");
                currentState = CROSS_FINISH_LINE;
                lastStateChangeTime = millis();
            }
            else if (line_follower())
            {
                lastStateChangeTime = millis();
                // line follower returned true, indicating that the line was found
            }
            
            break;
        }
        break;
    case AVOID_OBSTACLE:
        if (avoid_obstacle() == true)
        {
            //Serial.println("\nFOLLOW_LINE");
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
            finishTime = millis();
            obstacleTime = millis();
        }
        else if (millis() - lastStateChangeTime >= 100000)
        {
            // If 10 seconds have passed since last state change, switch back to FOLLOW_LINE
            //Serial.println("\nFOLLOW_LINE (timeout)");
            currentMovementState = STATE_SLEEP;
            // currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
        }
        break;
        break;
    case CROSS_FINISH_LINE:
        if (crossFinishLine() == true && finish_line_crossed == false)
        {
            finish_line_crossed = true;
            //Serial.println("\nFOLLOW_LINE");
            finishTime = millis();
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
        }
        else if (finish_line_crossed == true)
        {
            //Serial.println("\nFINISH");
            currentState = FINISH;
        }
        break;
    case RECOVER_FROM_NO_LINE:
        static int recoverAttempts = 0;
        if (recover() == true)
        {
            //Serial.println("\nFOLLOW_LINE");
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
            recoverAttempts = 0; // reset recover attempts counter
        }
        else if (recoverAttempts >= 3)
        {
            // If recover attempts exceed 3, switch back to FOLLOW_LINE
            Serial.println("\nFOLLOW_LINE (max recover attempts reached)");

            currentMovementState = STATE_SLEEP;
            lastStateChangeTime = millis();
            recoverAttempts = 0; // reset recover attempts counter
        }
        else
        {
            recoverAttempts++;
        }
        break;
    case FINISH:
        if (lastStateChangeTime > 1500)
        {
            // if this is the first time in the FINISH state, set the last state change time
            currentMovementState = STATE_SLEEP;
        }
        break;
    }
}
