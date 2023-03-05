// const char* ssid = "TomHagdorn";
// const char* password = "26071997";

const char* ssid = "Get off my Lan!";
const char* password = "SchroedingersChat";


unsigned long stateTime = 0;
unsigned long lastMovementChangeTime = 0;

uint32_t lastCamera = 0; ///< To store time value for repeated capture

// variables updated by nodered
//int pixel_threshold = 125;
int recover_time  = 5000;
int obst_stop_t = 700;
int obst_left_t =1000;
int obst_straight_t=1000;
int obst_right_t =1000;
int line_width=30;
int Fin_line_width=200;
int min_line_length = 100;
//define obstacle detection distance
int obstacle_detection_dist = 0;
int pixel_threshold = 30;



int cameraImageExposure = 2; ///< Camera exposure (0 - 1200) 2 for bright line
//! Image gain
/*!
    Range: (0 - 30)
    If gain and exposure both set to zero then auto adjust is enabled
*/
int cameraImageGain = 18;  //18 for bright line



//filter settings
const int kernelSize = 3; // kernel size for gaussian blur

//TODO Set the default values for the variables for
//strip.color(0,0,0);








