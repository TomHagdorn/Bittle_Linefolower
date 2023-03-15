const char* ssid = "TomHagdorn";
const char* password = "26071997";

// const char* ssid = "Get off my Lan!";
// const char* password = "SchroedingersChat";


unsigned long stateTime = 0;
unsigned long lastMovementChangeTime = 0;
uint32_t lastCamera = 0; ///< To store time value for repeated capture

// variables updated by nodered
int line_width=30;
int min_line_length = 130;
//define obstacle detection distance
int obstacle_detection_dist = 9;
int obstacle_tolerance = 4;
int pixel_threshold = 220;
unsigned long obstacle_forward_time = 1500;
unsigned long recover_time = 1000;



int cameraImageExposure = 6; ///< Camera exposure (0 - 1200) 2 for bright line
//! Image gain
/*!
    Range: (0 - 30)
    If gain and exposure both set to zero then auto adjust is enabled
*/
int cameraImageGain = 18;  //18 for bright line
//filter settings
const int kernelSize = 3; // kernel size for gaussian blur








