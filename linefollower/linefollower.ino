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

const int serialSpeed = 115200; ///< Serial data speed to use
// My files

# define BAUDRATE 115200
#include "nodeRed_variables.h"
#include "camera_setup.h"
#include "calibration.h"
#include "control.h"
#include "light_strip.h"
#include "node_red.h"
//#define PIN_S2RX 4 // Bittle RX on IO12
#define PIN_S2TX 4 // Bittle TX on IO13

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
static unsigned long startTime = millis();

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

        // start primary serial connection
    Serial.begin(BAUDRATE);

    // start secondary serial connection
    BittleSerial.begin(BAUDRATE, SERIAL_8N1, PIN_S2TX);
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
    setup_wifi();
    setupOnBoardFlash();
    setLedBrightness(ledBrightness);
    server.on("/Start", HandleClienttrue); // Wifi functions to start or stop the update()
    server.on("/Stop", HandleClientfalse);
    // Wifi functions to start or stop the update()
    //.on("/Stop_server", server_set_off);
    server.on("/status", handle_status);
    
    send_image();
    //update all ned_node values
    Update_node_red_values();
    // Ultrasound sensor setup
    strip_setup();

    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
}
/**************************************************************************/
/*!
  \brief  Loop function
  Captures and processes ca image every 320ms
*/
/**************************************************************************/
void loop()
{

    cycle_led_strip();

    if ((millis() - lastCamera > 300))
    {
        //unsigned long loopTimeStart = millis();
        esp_err_t res = camera_capture();

        if (res == ESP_OK )
        {
            //gaussianBlur(kernelSize);
            // sobel();
            threshold_image();
            // unsigned long thresholdTime = millis();
            //  Serial.print("Threshold time: ");
            //  Serial.println(thresholdTime - loopTimeStart);
            if (bstart == true){
              update();
              update_movement();
            }
            //update();
            // unsigned long updateTime = millis();
            //  Serial.print("Update time: ");
            //  Serial.println(updateTime - thresholdTime);
            //update_movement();
            //Serial.println(bstart);


            // unsigned long movementTime = millis();
            //  Serial.print("Movement time: ");
            //  Serial.println(movementTime - updateTime);

            // update server every 600ms to save resources
            if (millis() - lastServerUpdate >= 900)
            {
                server.handleClient();
            }

            // print image to serial monitor
            // capture_still();
            // free the sobel image gradient buffer
            // delete[] gradient;
            lastCamera = millis();
        }
        //unsigned long loopTimeEnd = millis();
        //Serial.print("Loop time: ");
        //Serial.println(loopTimeEnd - loopTimeStart);
        // return the frame buffer back to the driver for reuse
        esp_camera_fb_return(fb);
    }
    // Print the loop timeg
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

    switch (currentState)
    {
    case FOLLOW_LINE:
        if (detect_obstacle() == true)
        {
            Serial.println("\nAVOID_OBSTACLE");
            currentState = AVOID_OBSTACLE;
            lastStateChangeTime = millis();
        }
        else
        {
            if (line_follower())
            {
                lastStateChangeTime = millis();
                // line follower returned true, indicating that the line was found

                if (check_for_horizontal_line() && (millis() - startTime >= 60000))
                {
                    Serial.println("\nCROSS_FINISH_LINE");
                    currentState = CROSS_FINISH_LINE;
                    lastStateChangeTime = millis();
                }
            }

            // else
            // {
            //     if (millis() - lastStateChangeTime >= 10000)
            //     {
            //         // line follower returned false, indicating that the line was not found
            //         // and at least 3 seconds have passed since the last state change
            //         Serial.println("\nRECOVER_FROM_NO_LINE");
            //         currentState = RECOVER_FROM_NO_LINE;
            //         lastStateChangeTime = millis();
            //     }
            //     break;
            // }
            break;
        }
        break;
    case AVOID_OBSTACLE:
        if (avoid_obstacle() == true)
        {
            Serial.println("\nFOLLOW_LINE");
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
        }
        else if (millis() - lastStateChangeTime >= 100000)
        {
            // If 10 seconds have passed since last state change, switch back to FOLLOW_LINE
            Serial.println("\nFOLLOW_LINE (timeout)");
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
        }
        break;
        break;
    case CROSS_FINISH_LINE:
        if (crossFinishLine() == true && finish_line_crossed == false)
        {
            finish_line_crossed = true;
            cool_move();
            Serial.println("\nFOLLOW_LINE");
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
        }
        else
        {
            currentState = FINISH;
        }
        break;
    case RECOVER_FROM_NO_LINE:
        static int recoverAttempts = 0;
        if (recover() == true)
        {
            Serial.println("\nFOLLOW_LINE");
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
            recoverAttempts = 0; // reset recover attempts counter
        }
        else if (recoverAttempts >= 3)
        {
            // If recover attempts exceed 3, switch back to FOLLOW_LINE
            Serial.println("\nFOLLOW_LINE (max recover attempts reached)");
            currentState = FOLLOW_LINE;
            lastStateChangeTime = millis();
            recoverAttempts = 0; // reset recover attempts counter
        }
        else
        {
            recoverAttempts++;
        }
        break;
    case FINISH:
        Serial.print("kbalance");
        // Robot has finished, do nothing
        break;
    }
}
