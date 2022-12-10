#include <iostream>
#include <vector>

// Represents a point in an image
struct Point {
  int x;
  int y;
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

// Returns the point with the highest density of white pixels in the given image
Point findPointWithHighestDensity(const std::vector<Point>& image) {
  // Keep track of the maximum density and the point with the maximum density
  int maxDensity = 0;
  Point maxPoint = {-1, -1};

  // For each point in the image, count the number of points within a radius of 10 pixels
  for (const auto& point : image) {
    int density = 0;
    for (const auto& other : image) {
      if (abs(point.x - other.x) <= 10 && abs(point.y - other.y) <= 10) {
        ++density;
      }
    }

    // If the density of the current point is higher than the maximum density,
    // update the maximum density and the point with the maximum density
    if (density > maxDensity) {
      maxDensity = density;
      maxPoint = point;
    }
  }

  return maxPoint;
}

int main() {
  // Read the image from the input stream
  auto image = readImage(std::cin);

  // Find the point with the highest density of white pixels
  auto point = findPointWithHighestDensity(image);

  // Output the point with the highest density of white pixels
  std::cout << "Point with highest density: (" << point.x << ", " << point.y << ")" << std::endl;

  return 0;
}