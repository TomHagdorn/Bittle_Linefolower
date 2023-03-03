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
  HardwareSerial Serial2(2);
    if (Serial2.available()) {
      char received_char = Serial2.read();

      if (received_char == 'l') {
        return 0;
      } else if (received_char == 'r') {
        return 320;
      } else if (received_char == 'm') {
        return 180;
      } else if (received_char == 'n') {
        return -1;
      }
    }
    return -1;
 }
