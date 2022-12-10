#include <iostream>
#include <vector>

// Represents a point in an image
struct Point {
  int x;
  int y;
};

// Represents a line in an image
struct Line {
  Point start;
  Point end;
};

// Reads an image from the given input stream and returns a vector of points
// where the value of the pixel is 1 (white)
std::vector<Point> readImage(std::istream& input) {
  // Read the width and height of the image
  int width, height;
  input >> width >> height;

  std::vector<Point> points;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixel;
      input >> pixel;
      if (pixel == 1) {
        points.push_back({x, y});
      }
    }
  }

  return points;
}

// Detects lines in the given image and returns a vector of lines
std::vector<Line> detectLines(const std::vector<Point>& image) {
  std::vector<Line> lines;

  // TODO: Implement line detection algorithm

  return lines;
}

// Returns whether the line is on the left, right, or center of the image
std::string linePosition(const Line& line, int imageWidth) {
  // Compute the middle of the line
  int x = (line.start.x + line.end.x) / 2;

  // Check if the middle of the line is on the left, right, or center of the image
  if (x < imageWidth / 3) {
    return "left";
  } else if (x > 2 * imageWidth / 3) {
    return "right";
  } else {
    return "center";
  }
}

int main() {
  // Read the image from the input stream
  auto image = readImage(std::cin);

  // Detect lines in the image
  auto lines = detectLines(image);

  // Output the detected lines and their position
  std::cout << "Detected lines:" << std::endl;
  for (const auto& line : lines) {
    std::cout << "Start: (" << line.start.x << ", " << line.start.y << ") "
              << "End: (" << line.end.x << ", " << line.end.y << ") "
              << "Position: " << linePosition(line, image.width) << std::endl;
  }

  return 0;
}