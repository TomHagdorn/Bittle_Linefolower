// Pin definitions for ultrasound sensor
#define trigPin     12
#define echoPin     13



//TODO not enough memory unfortunately figure something out
//int* gradient = new int[fb->height * fb->width];

/**
 * @brief Applies a Gaussian blur to the image buffer.
 * 
 * @param kernelSize The size of the Gaussian kernel to use. This must be an odd integer.
 */
void gaussianBlur(int kernelSize) {
  float kernel[] = { 1, 2, 1, 2, 4, 2, 1, 2, 1 }; // 3x3 Gaussian kernel

  int radius = kernelSize / 2;   

  int height = fb->height;
  int width = fb->width;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      float sum = 0;
      float weight = 0;

      for (int ky = -radius; ky <= radius; ky++) {
        for (int kx = -radius; kx <= radius; kx++) {
          int px = x + kx;
          int py = y + ky;

          if (px >= 0 && px < width && py >= 0 && py < height) {
            float w = kernel[(ky + radius) * kernelSize + (kx + radius)];
            sum += w * fb->buf[py * width + px];
            weight += w;
          }
        }
      }

      fb->buf[y * width + x] = (uint8_t)(sum / weight);
    }
  }
}

/**
 * @brief Thresholds the image buffer to binary (black and white).
 * 
 * @note This function assumes that the frame buffer (`fb`) contains grayscale pixel values.
 * 
 * @param pixel_threshold The threshold value to use. Pixels with values greater than this will be set to 255 (white), 
 *                        while pixels with values less than or equal to this will be set to 0 (black).
 */
void threshold_image()
{
    // get the height and width of the frame
    int height = fb->height;
    int width = fb->width;

    

    // threshold the entire frame
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int index = row * width + col;

            // threshold the pixel at the current index
            // if the pixel is less than 210, set it to 255 (white)
            fb->buf[index] = (fb->buf[index] > pixel_threshold) ? 255 : 0;
        }
    }
}

/**
 * @brief the middle point of a white line in the specified region of the frame.
 *
 * @return The x-coordinate of the middle point of the white line, or -1 if no valid line is found.
 */
int get_middle_point()
{   
    double start_fraction = 0;
    double end_fraction = 1.0;
    // initialize the starting and ending x-coordinates to zero
    int start_x = 0;
    int end_x = 0;
    // flag to track if we have found the start of the white pixels
    bool found_start = false;
    // variable to track the number of consecutive non-white pixels
    int consecutive_non_white = 0;
    // variable to track the number of consecutive white pixels to check row validity
    int consecutive_white = 0;
    // row counter
    int valid_row_counter = 0;
    // vector to store the middle points of each row
    std::vector<int> middle_points;

    // calculate the start and end rows based on the start and end fractions
    int start_row = fb->height * start_fraction;
    int end_row = fb->height * end_fraction;
    // check if the start and end rows are valid
    if ((end_row > fb->height) || (start_row < 0) || (start_row > end_row))
    {
        // invalid start and end rows, so return -1
        Serial.println("Invalid start and end rows");
        Serial.println("Height: " + String(fb->height) + ", Start Row: " + String(start_row) + ", End Row: " + String(end_row));
        Serial.println("Need 0 <= start_row < end_row <= height");
        return -1;
    }
    // iterate over the rows in the specified range
    for (int y = start_row; y < end_row; y++)
    {
        // iterate over the columns in the current row
        for (int x = 0; x < fb->width; x++)
        {   
    
            // get the current pixel value
            uint8_t pixel = fb->buf[y * fb->width + x];

            // if the pixel is white (i.e., its value is above the threshold)
            if (pixel == 255)
            {
                // if we haven't found the start of the white pixels yet,
                // set the start x-coordinate to the current x-coordinate
                consecutive_white++;
                // reset the consecutive non-white pixels counter
                consecutive_non_white = 0;
                // if we haven't found the start of the white pixels yet,
                if (!found_start)
                {
                    start_x = x;
                    found_start = true;
                }
                // update the ending x-coordinate to the current x-coordinate
                end_x = x;
            }
            else
            {
                // increment the consecutive non-white pixels counter
                consecutive_non_white++;
            }
            // if we have seen 15 consecutive non-white pixels, set the end x-coordinate to
            // the current x-coordinate minus 15
            if (consecutive_non_white >= 8)
            {
                end_x = x - 15;
                consecutive_non_white = 0;
                found_start = false;
            }
            // if we have seen 40 consecutive white pixels, then we have found a valid row
            if (consecutive_white >= 20 && found_start)
            {
                valid_row_counter++;
                // calculate the middle x-coordinate of the white pixels
                int middle_point = (end_x + start_x) / 2;
                // add the middle point to the vector

                middle_points.push_back(middle_point);
            }
        }
    }
    // if we have found at least one valid row
    if (valid_row_counter > line_width)
    {
        // sort the middle points
        std::sort(middle_points.begin(), middle_points.end());
        // calculate the median middle point
        int median_middle_point = middle_points[middle_points.size() / 2];
        // return the median middle point
        return median_middle_point;
    }
    // if we haven't found any valid rows, return -1
    else
    {
        return -1;
    }
}

/**
 * @brief Gets the distance to the nearest object in front of the ultrasonic sensor.
 *  
 * @return The distance to the nearest object in front of the ultrasonic sensor.
*/
int get_distance()
{
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    int distance = duration * 0.034 / 2;
    // Prints the distance on the Serial Monitor
    //SSerial.print("Distance: ");
    //Serial.println(distance);
    return distance;
}

/**
 * @brief Checks if the robot is at the end of the line.
 * 
 * @return True if the robot is at the end of the line, false otherwise.
*/
bool check_for_horizontal_line()
{
    // calculate the start and end rows of the lowest third of the image
    int start_row = fb->height * 2 / 3;
    int end_row = fb->height - 1;

    // counter for the number of lines found
    int line_count = 0;

    // iterate over the rows in the lowest third of the image
    for (int y = start_row; y <= end_row; y++)
    {
        // initialize the consecutive white pixel count to zero
        int white_pixel_count = 0;

        // iterate over the pixels in the current row
        for (int x = 0; x < fb->width; x++)
        {
            // get the current pixel value
            uint8_t pixel = fb->buf[y * fb->width + x];

            // if the pixel is white (i.e., its value is above the threshold)
            // then increment the consecutive white pixel count
            if (pixel == 255)
            {
                white_pixel_count++;
            }
            // if the pixel is not white (i.e., its value is below the threshold)
            // then reset the consecutive white pixel count
            else
            {
                white_pixel_count = 0;
            }

            // if there are at least min_line_length consecutive white pixels
            // then increment the line count
            if (white_pixel_count >= min_line_length)
            {
                line_count++;
                break;
            }
        }
    }

    // if three or more lines were found, return true
    if (line_count >= 3)
    {
        return true;
    }
    // otherwise, return false
    else
    {
        return false;
    }
    //return false;
}





// void threshold_gradient() {
//     // get the height and width of the frame
//     int height = fb->height;
//     int width = fb->width;

//     // threshold the entire frame using the gradient magnitude
//     for (int row = 0; row < height; row++)
//     {
//         for (int col = 0; col < width; col++)
//         {
//             int index = row * width + col;

//             // compare the magnitude of the gradient at the current index to the threshold
//             int mag = gradient[index];
//             // adding this print somehow causes it to break
//             //Serial.println(mag  +"\t" + threshold);
//             if (mag < pixel_threshold) {
//                 // if the magnitude is less than the threshold, set the pixel to white (255)
//                 fb->buf[index] = 255;
//             } else {
//                 // otherwise, set the pixel to black (0)
//                 fb->buf[index] = 0;
//             }
//         }
//     }
    
// }

//not fixable it just alocates more memorz than it can handle
//memory leak or smt 
/*
ELF file SHA256: 0000000000000000

Rebooting...
ets Jul 29 2019 12:21:46

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13864
load:0x40080400,len:3608
entry 0x400805f0
CORRUPT HEAP: Bad head at 0x3f813498. Expected 0xabba1234 got 0x3f800014

assert failed: multi_heap_free multi_heap_poisoning.c:253 (head != NULL)


Backtrace:0x40083ef1:0x3ffb25600x4008a639:0x3ffb2580 0x4008fed5:0x3ffb25a0 0x4008fb37:0x3ffb26d0 0x40084231:0x3ffb26f0 0x4008ff05:0x3ffb2710 0x4010246d:0x3ffb2730 0x40102a75:0x3ffb2750 0x400d2ec5:0x3ffb2770 0x400d3948:0x3ffb2800 0x400d9875:0x3ffb2820 

*/
// void sobel() {

//   int sobelX[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 }; // Sobel operator for x direction
//   int sobelY[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 }; // Sobel operator for y direction

//   int radius = 1;

//   int height = fb->height;
//   int width = fb->width;

   
//   for (int y = radius; y < height - radius; y++) {
//     for (int x = radius; x < width - radius; x++) {
//       int sumX = 0;
//       int sumY = 0;

//       for (int ky = -radius; ky <= radius; ky++) {
//         for (int kx = -radius; kx <= radius; kx++) {
//           int px = x + kx;
//           int py = y + ky;

//           int index = (ky + radius) * (radius * 2 + 1) + (kx + radius);
//           int value = fb->buf[py * width + px];

//           sumX += sobelX[index] * value;
//           sumY += sobelY[index] * value;
//         }
//       }

//       gradient[y * width + x] = abs(sumX) + abs(sumY); // Compute the gradient magnitude
//     }
//     delete[] gradient; // Free memory allocated for gradient array
//   }
// }
