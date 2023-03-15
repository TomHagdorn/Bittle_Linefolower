/**
 * @file testing.h
 * @brief file containing functions to test the robot by sending values via serial
 * @author Tom Hagdorn
 */


/**
 * @brief Captures and prints the lowest third of the image.
 * 
 * This function captures the lowest third of the image and prints the pixel values of each
 * pixel in the console. The function returns true to indicate that the still was successfully
 * captured.
 *
 * @return True if the still was successfully captured, false otherwise.
 */
bool capture_still()
{
    // Calculate the height of the lowest third of the image
    int third_height = IMAGE_HEIGHT / 3;

    // Iterate over the rows in the lowest third of the image
    for (int row_index = 2 * third_height; row_index < IMAGE_HEIGHT; ++row_index)
    {
        // Iterate over the columns in the current row
        for (int col_index = 0; col_index < IMAGE_WIDTH; ++col_index)
        {
            // Print the value of the current pixel
            Serial.print(fb->buf[IMAGE_WIDTH * row_index + col_index]);
            Serial.print(" ");
        }
        // Move to the next line after printing the values for the current row
        Serial.print("\n");
    }
    Serial.println();
    // Return true to indicate that the still was successfully captured
    return true;
}


/**
 * @brief Gets the middle point test value from the Serial Monitor.
 *
 * This function reads a string from the Serial Monitor and returns an integer value based on
 * the received string. If the received string is "l", the function returns 0; if it is "r",
 * the function returns 320; if it is "m", the function returns 180; if it is "n", the function
 * returns -1. If no data is available on the Serial Monitor, the function returns -1.
 *
 * @return The middle point test value, or -1 if no data is available on the Serial Monitor.
 */
int get_middlepoint_test() {
  if (Serial.available() > 0) { // check if there is data available on the Serial Monitor
    String received_message = Serial.readString(); // read the complete message
    if (received_message == "l") {
      return 0;
    } else if (received_message == "r") {
      return 320;
    } else if (received_message == "m") {
      return 180;
    } else if (received_message == "n") {
      return -1;
    }
  }
  return -1;
}
int distance_test_value = 40;


/**
 * @brief Gets the distance test value from the Serial Monitor.
 *
 * This function reads a string from the Serial Monitor and returns an integer value based on
 * the received string. If the received string is "ob", the function sets the distance_test_value
 * to 8 and returns it; if the received string is "nob", the function sets the distance_test_value
 * to 40 and returns it. If no data is available on the Serial Monitor, the function returns the
 * current value of distance_test_value.
 *
 * @return The distance test value, or the current value of distance_test_value if no data is
 *         available on the Serial Monitor.
 */
int get_distance_test() {
  if (Serial.available() > 0) { // check if there is data available on the Serial Monitor
    String received_message = Serial.readString(); // read the complete message
    if (received_message == "ob") {
      distance_test_value = 8;
      return distance_test_value;
    }
    if (received_message == "nob") {
      distance_test_value = 40;
      return distance_test_value;
    }
  }
  return distance_test_value;
}