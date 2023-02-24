#include "board_sensor_setup.ino"
#include "img_proc_sensordata.ino"
#include "node_red.ino"
#include "calibration.ino"


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
    double start_fraction = 3.0 / 4.0;
    double end_fraction = 1.0;

    // get the middle point for the 3/4 to 1 portion of the frame
    int middle_point = get_middle_point(fb, start_fraction, end_fraction);
    if (middle_point == -1)
    {
        // no line was found, so stop the robot
        Serial.print("kp");
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


//TODO Upgrade to the new ultrasonic sensor
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
    double start_fraction_top = 0.0;
    double end_fraction_top = 1.0 / 3.0;
    double start_fraction_middle = 1.0 / 3.0;
    double end_fraction_middle = 2.0 / 3.0;
    double start_fraction_bottom = 2.0 / 3.0;
    double end_fraction_bottom = 1.0;

    // get the middle points for the top, middle, and bottom thirds of the frame
    int middle_point_top = get_middle_point(fb, start_fraction_top, end_fraction_top);
    int middle_point_middle = get_middle_point(fb, start_fraction_middle, end_fraction_middle);
    int middle_point_bottom = get_middle_point(fb, start_fraction_bottom, end_fraction_bottom);

    // if the middle point for the bottom third of the frame is not -1 (i.e., a valid row was found),
    // and the middle points for the top and middle thirds are -1 (i.e., no valid rows were found),
    // then we consider an obstacle to have been detected
    if (middle_point_bottom != -1 && middle_point_top == -1 && middle_point_middle == -1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//TODO fix this function
void avoid_obstacle() {
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

/**
 * @brief Makes the robot walk backwards for a certain amount of time to recover from losing the line.
 * 
 * @return void
 */
void recover()
{
    unsigned long start_time = millis();
    // walk backwards for a certain amount of time
    Serial.print("kwkB");
    while(millis() - start_time < 1000) {
    }
    Serial.print("kp");
    return;
}

/**
 * @brief Performs a cool move to celebrate crossing the finish line.
 * 
 * @return void
 */
void cool_move()
{
    // TODO David writes cool move and then we test
    //write function to turn on the spot
    return;
}

/**
 * @brief Makes the robot walk backwards for a certain amount of time after crossing the finish line.
 * 
 * @return void
 */
void crossFinishLine()
{
    // walk backwards for a certain amount of time
    Serial.print("kwkF");
    unsigned long start_time = millis();
    while (millis() - start_time < 400)
    {
    }
    Serial.print("kp");
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
            if (detect_obstacle(fb) && millis() - lastStateChangeTime >= 9000)
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