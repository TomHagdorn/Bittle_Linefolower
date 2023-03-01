#include "sensor_img_proc.h"

enum MovementState
{
    STATE_STOP,
    STATE_TURN_LEFT,
    STATE_TURN_RIGHT,
    STATE_MOVE_FORWARD,
    STATE_MOVE_BACKWARD,
    STATE_TURN_BACK_RIGHT,
    STATE_TURN_BACK_LEFT,
};

MovementState currentMovementState = STATE_STOP;
MovementState prevMovementState = STATE_MOVE_FORWARD;

//define state change time
static unsigned long lastStateChangeTime = 0;



/**************************************************************************/
/**
  Line Follower
  Follow a line using the camera
  \param fb: pointer to the frame buffer
  \return true if a line is found, false otherwise
 */
/**************************************************************************/
bool line_follower()
{
    // calculate the starting and ending fractions for the 3/4 to 1 portion of the frame
    double start_fraction = 3.0 / 4.0;
    double end_fraction = 1.0;
    // get the middle point for the 3/4 to 1 portion of the frame
    int middle_point = get_middle_point(start_fraction, end_fraction);
    if (middle_point == -1)
    {
        // no line was found, so stop the robot
        currentMovementState = STATE_STOP;
        return false;
    }

    // if the point of highest density is in one of the 3/7th of the left side of the picture
    if (middle_point < fb->width * 4 / 11)
    {
        // move the robot to the left
        currentMovementState = STATE_TURN_LEFT;
        return true;
    }
    // if the point of highest density is in one of the 3/7th of the right side of the picture
    else if (middle_point >= fb->width * 8 / 11)
    {
        // move the robot to the right
        currentMovementState = STATE_TURN_RIGHT;
        return true;
    }
    // if the point of highest density is within the 4/7th in the middle
    else
    {
        // move the robot forward
        currentMovementState = STATE_MOVE_FORWARD;
        return true;
    }
}


bool detect_obstacle() { 
    //TODO fix this function
    //Serial.println("detecting obstacle");
    if (get_distance() < obstacle_detection_dist && get_distance() != -1) {
        return true;
    }
    return false;
}

bool avoid_obstacle() {
    //stop the robot
    if (millis() - lastStateChangeTime < 500) {
        currentMovementState = STATE_STOP;
    }
    //turn left
    else if (millis() - lastStateChangeTime < 1000 && millis() - lastStateChangeTime > 500) {
        currentMovementState = STATE_TURN_LEFT;
    }
    //turn right
    else if (millis() - lastStateChangeTime < 1500 && millis() - lastStateChangeTime > 1000) {
        currentMovementState = STATE_TURN_RIGHT;
    }
    //move forward
    else if (millis() - lastStateChangeTime < 2000 && millis() - lastStateChangeTime > 1500) {
        currentMovementState = STATE_MOVE_FORWARD;
    }
    //turn right
    else if (millis() - lastStateChangeTime < 2500 && millis() - lastStateChangeTime > 2000) {
        currentMovementState = STATE_TURN_RIGHT;
    }
    //turn left
    else if (millis() - lastStateChangeTime < 3000 && millis() - lastStateChangeTime > 2500) {
        currentMovementState = STATE_TURN_LEFT;
    }
    //if line is found
    else if (millis() - lastStateChangeTime < 3500) {
        //TODO mabye check if line is found
        currentMovementState = STATE_STOP;
        return true;
    }
    return false;
}

bool recover()
{   
    //Walk backwards for a certain amount of time
    if (millis() - lastStateChangeTime < 1000) {
        currentMovementState = STATE_MOVE_BACKWARD;
    }
    else{
        
        return true;
    }
    return false;
}

void cool_move()
{
    // TODO David writes cool move and then we test
    //write function to turn on the spot
    return;
}

bool crossFinishLine()
{
    //walk forward for a certain amount of time then stop
    if (millis() - lastStateChangeTime < 500) {
        currentMovementState = STATE_MOVE_FORWARD;
    }
    else {
        
        return true;
    }
    return false;
}


void update_movement(){
    if (currentMovementState != prevMovementState) {
        switch (currentMovementState) {
            case STATE_STOP:
                Serial.print("kbalance");
                break;

            case STATE_TURN_LEFT:
                Serial.print("kwkL");
                break;

            case STATE_TURN_RIGHT:
                Serial.print("kwkR");
                break;

            case STATE_MOVE_FORWARD:
                Serial.print("kwkF");
                break;

            case STATE_TURN_BACK_RIGHT:
                Serial.print("kwkR");
                break;

            case STATE_TURN_BACK_LEFT:
                Serial.print("kwkL");
                break;

            case STATE_MOVE_BACKWARD:
                Serial.print("kwkB");
                break;
        }
        prevMovementState = currentMovementState;
    }
}
