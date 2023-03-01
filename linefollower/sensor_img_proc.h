// Pin definitions for ultrasound sensor
#define trigPin     12
#define echoPin     13

//filter settings
const int kernelSize = 3; // kernel size for gaussian blur

// poiner to the sobel gradient
//int *gradient;


void gaussianBlur(camera_fb_t *fb, int kernelSize) {
  float kernel[] = { 1, 2, 1, 2, 4, 2, 1, 2, 1 }; // 3x3 Gaussian kernel

  //TODO doesnt make sense is not an int
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

void sobel(camera_fb_t *fb, int *gradient) {
  int sobelX[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 }; // Sobel operator for x direction
  int sobelY[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 }; // Sobel operator for y direction

  int radius = 1;

  int height = fb->height;
  int width = fb->width;

  for (int y = radius; y < height - radius; y++) {
    for (int x = radius; x < width - radius; x++) {
      int sumX = 0;
      int sumY = 0;

      for (int ky = -radius; ky <= radius; ky++) {
        for (int kx = -radius; kx <= radius; kx++) {
          int px = x + kx;
          int py = y + ky;

          int index = (ky + radius) * (radius * 2 + 1) + (kx + radius);
          int value = fb->buf[py * width + px];

          sumX += sobelX[index] * value;
          sumY += sobelY[index] * value;
        }
      }

      gradient[y * width + x] = abs(sumX) + abs(sumY); // Compute the gradient magnitude
    }
  }
}
//TODO kust include the global variable pixel_threshold insted giving the function a threshold
void threshold_gradient(camera_fb_t *fb, int threshold, int* gradient) {
    // get the height and width of the frame
    int height = fb->height;
    int width = fb->width;

    // threshold the entire frame using the gradient magnitude
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int index = row * width + col;

            // compare the magnitude of the gradient at the current index to the threshold
            int mag = gradient[index];
            // adding this print somehow causes it to break
            //Serial.println(mag  +"\t" + threshold);
            if (mag < threshold) {
                // if the magnitude is less than the threshold, set the pixel to white (255)
                fb->buf[index] = 255;
            } else {
                // otherwise, set the pixel to black (0)
                fb->buf[index] = 0;
            }
        }
    }
}

//TODO Implement the newer functions later on. 
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
            fb->buf[index] = (fb->buf[index] < pixel_threshold) ? 255 : 0;
        }
    }
}



/**
 * @brief Finds the middle point of the white pixels in a region of the camera frame buffer.
 *
 * The function scans the specified region of the camera frame buffer for white pixels and calculates
 * the middle point of the white pixels. The middle point is determined by averaging the starting and
 * ending x-coordinates of the white pixels. The function also checks for the validity of the row
 * by checking for consecutive white pixels.
 *
 * @param fb The camera frame buffer.
 * @param start_fraction The fraction of the frame buffer's height at which to start scanning for white pixels.
 * @param end_fraction The fraction of the frame buffer's height at which to end scanning for white pixels.
 * @return The middle point of the white pixels in the specified region of the frame buffer, or -1 if no valid
 *         white pixels were found.
 */
int get_middle_point(double start_fraction, double end_fraction)
{   
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

/**************************************************************************/
/**
  Detect Obstacle
  Detect an obstacle using the camera
  \param fb: pointer to the frame buffer
  \return true if an obstacle is found, false otherwise
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
    // Serial.print("Distance: ");
    // Serial.println(distance);
    return distance;
}

/**************************************************************************/
/**
  Check for Horizontal Line
  Check if there is a horizontal line in the bottom third of the image
  \param fb: pointer to the frame buffer
  \return true if there is a horizontal line, false otherwise
 */
/**************************************************************************/
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

//Prepossessing function for the image using gaussian blur sobel filter and thresholding

