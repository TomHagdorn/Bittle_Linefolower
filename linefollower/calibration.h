bool capture_still(camera_fb_t *fb)
{   
    int height = fb->height;
    int width = fb->width;

    for (int row_index = height; row_index < height; ++row_index)
    {
        // Iterate over the columns in the current row
        for (int col_index = 0; col_index < width; ++col_index)
        {
            // Print the value of the current pixel
            Serial.print(fb->buf[width * row_index + col_index]);
            Serial.print(" ");
        }
        // Move to the next line after printing the values for the current row
        Serial.print("\n");
    }
    Serial.println();
    // Return true to indicate that the still was successfully captured
    return true;
}
