// void cycle_colors(int red_pin, int green_pin, int blue_pin)
// {
//   // Loop through all possible color values
//   for (int i = 0; i < 255; i++) {
//     analogWrite(red_pin, i);
//     analogWrite(green_pin, 255 - i);
//     analogWrite(blue_pin, i/2);

//     delay(10); // wait a bit before changing color
//   }
// }

// int find_best_color(int red_pin, int green_pin, int blue_pin)
// {
//   int best_color = 0;
//   double best_contrast = -1;

//   // Loop through all possible colors
//   for (int i = 0; i < 255; i++) {
//     // Set the RGB values to the current color
//     analogWrite(red_pin, i);
//     analogWrite(green_pin, 255 - i);
//     analogWrite(blue_pin, i/2);

//     // Calculate the contrast between the line and the surrounding pixels
//     double contrast = calculate_contrast(fb);

//     // If the contrast is better than the current best, update the best color and contrast
//     if (contrast > best_contrast) {
//       best_color = i;
//       best_contrast = contrast;
//     }
//   }

//   // Return the best color
//   return best_color;
// }

// /**
//  * Calculates the contrast between line and background in a grayscale image
//  * @param fb Pointer to the camera frame buffer
//  * @return The color with the highest contrast between line and background
//  */
// int calculate_contrast(const camera_fb_t *fb)
// {
//     // Initialize the count of white and black pixels
//     int white_count = 0;
//     int black_count = 0;

//     // Get the pixel buffer
//     uint8_t *pixel_buffer = fb->buf;

//     // Loop over all pixels in the image
//     for (int i = 0; i < fb->width * fb->height; i++)
//     {
//         // Get the current pixel value
//         uint8_t pixel_value = pixel_buffer[i];

//         // Increment the count of black or white pixels
//         if (pixel_value > 128)     //TODO write a calibration function for the thresholding 
//         {
//             white_count++;
//         }
//         else
//         {
//             black_count++;
//         }
//     }

//     // Calculate the contrast
//     uint32_t contrast = abs(white_count - black_count);

//     //return the contrast
// }
