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