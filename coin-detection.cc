#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

// Template matching function to identify coins
string identifyCoin(int radius) {
    if (radius >= 30 && radius <= 40) {
        return "Nickel";
    }
    else if (radius >= 20 && radius <= 25) {
        return "Dime";
    }
    // Add other coins here, such as penny, quarter, etc.
    return "Unknown Coin";
}

int main() {
    string imagePath = "/Users/mayajamaddar/Documents/coin-recognition-main/img/multiple-coin.jpg"; // Include your image path
    Mat image = imread(imagePath, IMREAD_COLOR);

    if (image.empty()) { // Error handling
        cout << "Could not read the image: " << imagePath << endl;
        return 1;
    }

    // Convert image to grayscale
    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);

    // Apply Gaussian Blur to blur the pattern
    Mat finalBlurredImage;
    GaussianBlur(grayImage, finalBlurredImage, Size(7, 7), 2, 2);

    // Perform Hough Circle Transform
    vector<Vec3f> circles;

    // Note: param2 adjustment is needed based on image size
    HoughCircles(finalBlurredImage, circles, HOUGH_GRADIENT, 1, finalBlurredImage.rows/8, 250, 30, 0, 0);

    Mat circleImage = image.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // Print the radius value to debug
        cout << "Detected circle radius: " << radius << endl;

        // Calculate circularity
        double perimeter = 2 * CV_PI * radius;
        double area = CV_PI * radius * radius;
        double circularity = (4 * CV_PI * area) / (perimeter * perimeter);

        // Filter circles based on radius and circularity threshold
        int radius_threshold_min = 0;
        int radius_threshold_max = 2000;
        double circularity_threshold = 0.7;

        if (radius >= radius_threshold_min && radius <= radius_threshold_max && circularity > circularity_threshold) {
            // Identify the coin type based on the radius
            string coinType = identifyCoin(radius);

            // Draw circles detected
            circle(circleImage, center, radius, Scalar(0, 255, 0), 3, 8, 0); // Green border for detected coins

            // Put text label
            putText(circleImage, coinType, Point(center.x - radius, center.y - radius - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
        }
    }

    // Display the images
    namedWindow("Original Image", WINDOW_AUTOSIZE);
    imshow("Original Image", image);

    namedWindow("Grayscale Image", WINDOW_AUTOSIZE);
    imshow("Grayscale Image", grayImage);

    namedWindow("Final Blurred Image", WINDOW_AUTOSIZE);
    imshow("Final Blurred Image", finalBlurredImage);

    namedWindow("Detected Circles", WINDOW_AUTOSIZE);
    imshow("Detected Circles", circleImage);

    waitKey(0);
    return 0;
}
