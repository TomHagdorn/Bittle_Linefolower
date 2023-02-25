#include "sensor_img_proc.h"

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
    while(millis() - start_time < 1000) {
    }
    Serial.print("kbalanceI");
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