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
    STATE_SLEEP,
    STATE_HI
};

MovementState currentMovementState = STATE_STOP;
MovementState prevMovementState = STATE_MOVE_FORWARD;
static int obst_state = 0;
static unsigned long obst_stateStartTime = 0;

// defobst_state change time
static unsigned long lastStateChangeTime = 0;
static unsigned long finishTime = 0;
 int counter = 0;

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
    // get the middle point for the 3/4 to 1 portion of the frame
    //int middle_point = get_middle_point();
    int middle_point = get_middle_point();
    if (middle_point == -1)
    {
        // no line was found, so stop the robot
        currentMovementState = STATE_MOVE_BACKWARD;
        return false;
    }

    // if the point of highest density is in one of the 3/7th of the left side of the picture
    if (middle_point < fb->width * 1 / 4)
    {
        // move the robot to the left
        currentMovementState = STATE_TURN_LEFT_AXIS;
        return true;
    }
    // if the point of highest density is in one of the 3/7th of the right side of the picture
    else if (middle_point >= fb->width * 3 / 4)
    {
        // move the robot to the right
        currentMovementState = STATE_TURN_RIGHT_AXIS;
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
    //if (get_distance_test() < obstacle_detection_dist)
    if (get_distance() < obstacle_detection_dist)
    {
        return true;
    }
    return false;
}

bool avoid_obstacle()
{
    switch (obst_state)
    {
    case 0: // Turn left
        if (get_distance() > obstacle_detection_dist + obstacle_tolerance )
        {
            obst_state = 1;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_TURN_LEFT_AXIS;
        break;

    case 1: // Walk forward
        if (millis() - obst_stateStartTime >= obstacle_forward_time)
        {
            obst_state = 2;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_MOVE_FORWARD;
        break;

    case 2: // TURN RIGHT
        if (get_distance() < obstacle_detection_dist + obstacle_tolerance )
        {
            obst_state = 3;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_TURN_RIGHT_AXIS;
        break;


    case 3: // Turn left
        if (get_distance() > obstacle_detection_dist + obstacle_tolerance )
        {   
            counter = counter + 1;
            if (counter >= 2)
            {
                obst_state = 4;
                obst_stateStartTime = millis();
            }

        }
        currentMovementState = STATE_TURN_LEFT_AXIS;
        break;
    case 4: // Turn left
        if (any_line_found() == true)
        {
            obst_state = 5;
            obst_stateStartTime = millis();
        }
        currentMovementState = STATE_MOVE_FORWARD;
        break;
    case 5: // Turn left
        if (get_middle_point() != -1 || check_for_horizontal_line() == true)
        {
            
            obst_stateStartTime = millis();
            obst_state = 0;
            return true;
        }
        currentMovementState = STATE_TURN_LEFT;
        break;
    }
    

    return false;
    
}

bool recover()
{
    // // Rotate to find line again
    // if (millis() - lastStateChangeTime < recover_time)
    // {
    //     if (millis() - lastStateChangeTime < recover_time / 4 && get_middle_point() == -1)
    //     {
    //         currentMovementState = STATE_TURN_RIGHT_AXIS;
    //     }
    //     else if (millis() - lastStateChangeTime < recover_time * 2 / 4 && get_middle_point() == -1)
    //     {
    //         currentMovementState = STATE_TURN_LEFT_AXIS;
    //     }
    //     else if (millis() - lastStateChangeTime < recover_time * 3 / 4 && get_middle_point() == -1)
    //     {
    //         currentMovementState = STATE_TURN_RIGHT_AXIS;
    //     }
    //     else if (millis() - lastStateChangeTime < recover_time * 4 / 4 && get_middle_point() == -1)
    //     {
    //         currentMovementState = STATE_TURN_LEFT_AXIS;
    //     }
    //     else if (get_middle_point() != -1)
    //     {
    //         return true;
    //     }
    //     else
    //     {
    //         return false;
    //         Serialprintln("Recovery failed");
    //     }

    // }      
        return true;
}

void cool_move()
{
    
    return;
}

bool crossFinishLine()
{
    // walk forward for a certain amount of time then stop
    currentMovementState = STATE_MOVE_FORWARD;
    // if (millis() - lastStateChangeTime > 2000)
    // {
        currentMovementState = STATE_HI;

        if (millis() - lastStateChangeTime > 5000)
        {
            
            currentMovementState = STATE_TURN_LEFT_AXIS;//TODO oder timer!
            if (check_for_horizontal_line() == true | get_middle_point() != -1)
            {
                currentMovementState = STATE_MOVE_FORWARD;
                finishTime = millis();
                return true;
            }

        // }
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
            Serial.print("kvtR");
            lastMovementChangeTime = millis();
            break;
        
        case STATE_TURN_LEFT_AXIS:
            Serial.print("kvtL");
            lastMovementChangeTime = millis();
            break;
        case STATE_SLEEP:
            Serial.print("ksleep");
            lastMovementChangeTime = millis();
            break;
        case STATE_HI:
            Serial.print("khi");
            lastMovementChangeTime = millis();
            break;
        }
        prevMovementState = currentMovementState;
    }
}
