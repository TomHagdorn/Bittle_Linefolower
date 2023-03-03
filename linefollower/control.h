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
    STATE_TURN_RIGHT_AXIS,
    STATE_TURN_LEFT_AXIS,
};

MovementState currentMovementState = STATE_STOP;
MovementState prevMovementState = STATE_MOVE_FORWARD;
static int obst_state = 0;
static unsigned long obst_stateStartTime = 0;

// defobst_state change time
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

bool detect_obstacle()
{
    if (get_distance() < obstacle_detection_dist && get_distance() != -1)
    {
        return true;
    }
    return false;
}

bool avoid_obstacle()
{

    switch (obst_state)
    {
    case 0: // Stop for obstacle
        if (millis() - obst_stateStartTime >= obst_stop_t)
        {
            obst_state = 1;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_STOP;
        break;

    case 1: // Turn left
        if (get_distance() > obstacle_detection_dist + 5)
        {
            obst_state = 2;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_TURN_LEFT_AXIS;
        break;

    case 2: // Walk forward
        if (millis() - obst_stateStartTime >= obst_right_t)
        {
            obst_state = 3;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_MOVE_FORWARD;
        break;

    case 3: // TURN RIGHT
        if (get_distance() > obstacle_detection_dist + 10)
        {
            obst_state = 4;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_TURN_RIGHT_AXIS;
        break;

    case 4: // Turn left
        if (get_distance() > obstacle_detection_dist + 7)
        {
            obst_state = 5;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_TURN_RIGHT_AXIS;
        break;

    case 5: // Walk forward
        if (any_line_found() == true)
        {
            obst_state = 6;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_TURN_LEFT;
        break;

    case 6: // Turn left
        if (get_middle_point() == -1)
        {
            obst_state = 7;
            obst_stateStartTime = millis();
            return true;
        }
        currentMovementState = STATE_TURN_LEFT;
        break;
    }
    

    return false;
    
}

bool recover()
{
    // Walk backwards for a certain amount of time
    if (millis() - lastStateChangeTime < 1000)
    {
        currentMovementState = STATE_MOVE_BACKWARD;
    }
    else
    {

        return true;
    }
    return false;
}

void cool_move()
{
    // TODO David writes cool move and then we test
    // write function to turn on the spot
    return;
}

bool crossFinishLine()
{
    // walk forward for a certain amount of time then stop
    if (millis() - lastStateChangeTime < 500)
    {
        currentMovementState = STATE_MOVE_FORWARD;
    }
    else
    {

        return true;
    }
    return false;
}

void update_movement()
{
    if (currentMovementState != prevMovementState)
    {
        switch (currentMovementState)
        {
        case STATE_STOP:
            Serial.print("kbalance");
            lastMovementChangeTime = millis();
            break;

        case STATE_TURN_LEFT:
            Serial.print("kwkL");
            lastMovementChangeTime = millis();
            break;

        case STATE_TURN_RIGHT:
            Serial.print("kwkR");
            lastMovementChangeTime = millis();
            break;

        case STATE_MOVE_FORWARD:
            Serial.print("kwkF");
            lastMovementChangeTime = millis();
            break;

        case STATE_TURN_BACK_RIGHT:
            Serial.print("kwkR");
            lastMovementChangeTime = millis();
            break;

        case STATE_TURN_BACK_LEFT:
            Serial.print("kwkL");
            lastMovementChangeTime = millis();
            break;

        case STATE_MOVE_BACKWARD:
            Serial.print("kwkB");
            lastMovementChangeTime = millis();
            break;
        
        case STATE_TURN_RIGHT_AXIS:
            Serial.print("kwkRA");
            lastMovementChangeTime = millis();
            break;
        
        case STATE_TURN_LEFT_AXIS:
            Serial.print("kwkLA");
            lastMovementChangeTime = millis();
            break;
        }
        prevMovementState = currentMovementState;
    }
}
