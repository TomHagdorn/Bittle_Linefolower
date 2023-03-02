unsigned long stateTime = 0;

uint32_t lastCamera = 0; ///< To store time value for repeated capture

// variables updated by nodered
//int pixel_threshold = 125;
int recover_time  = 1000;
int obst_left_t =1000;
int obst_straight_t=1000;
int obst_right_t =1000;
int line_width=10;
int Fin_line_width=20;
int min_line_length = 200;
//define obstacle detection distance
int obstacle_detection_dist = 15;
int pixel_threshold = 180;

//TODO Set the default values for the variables for
//strip.color(0,0,0);

//possible states of the robot







