#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

int main() {
    string imagePath = "C:/Users/rdire/Documents/coin-recognition/img/coins2.jpg"; //Include your image path
    Mat image = imread(imagePath, IMREAD_COLOR);

    if (image.empty()) { //Error handling
        cout << "Could not read the image: " << imagePath << endl;
        return 1;
    }

    //Convert image to grayscale
    Mat grayImage;
    cvtColor(image, grayImage, COLOR_BGR2GRAY);

    //Apply Gaussian Blur to blur the pattern
    Mat finalBlurredImage;
    GaussianBlur(grayImage, finalBlurredImage, Size(7, 7), 2, 2); 

    // //Apply Canny Edge detection to extract edge map
    // Mat edgeImage;
    // //50 = lower threshold so ignore everything below, 150 = higher threshold so everything above 150 is strong edge
    // Canny(finalBlurredImage, edgeImage, 50, 150);

    //Perform Hough Circle Transform
    //Vector to store the circle, represents circle in three float values, centerX, centerY, radius
    vector<Vec3f> circles;

    /*
    Input image
    A vector that stores 3 values: x,y, r for each circle.
    HOUGH_GRADIENT: method to detect circle
    param1: The inverse ratio of resolution, 1 means accumulator has same resolution as input image
    param2: Minimum distance between centers (to prevent overlapping circles from being detected)
    param3: Upper threshold for Canny edge detector
    param4: Threshold for center detection. A smaller value allows for detection of more circles
    param5: The minimum radius of circles to detect, 0 for default
    param6: the maximum radius of circles to detect, 0 for default
    */

   //Note: param2 adjustment is needed based on image size
    HoughCircles(finalBlurredImage, circles, HOUGH_GRADIENT, 1, finalBlurredImage.rows/8, 250, 30, 0, 0); 

    Mat circleImage = image.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        //Calculate circularity
        double perimeter = 2 * CV_PI * radius;
        double area = CV_PI * radius * radius;
        double circularity = (4 * CV_PI * area) / (perimeter * perimeter);

        //Filter circles based on radius and circularity threshold
        int radius_threshold_min = 0;
        int radius_threshold_max= 2000;
        double circularity_threshold = 0.7;

        if (radius >= radius_threshold_min && radius <= radius_threshold_max && circularity > circularity_threshold){

            //Draw circles detected
            circle(circleImage, center, radius, Scalar(0, 255, 0), 3, 8, 0); //Green border for detected coins

            //Put text label
            putText(circleImage, "Coin", Point(center.x - radius, center.y - radius - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
        }
    }

    //Display the images
    namedWindow("Original Image", WINDOW_AUTOSIZE);
    imshow("Original Image", image);

    namedWindow("Grayscale Image", WINDOW_AUTOSIZE);
    imshow("Grayscale Image", grayImage);

    // namedWindow("Enhanced Image", WINDOW_AUTOSIZE);
    // imshow("Enhanced Image", enhancedImage);

    namedWindow("Final Blurred Image", WINDOW_AUTOSIZE);
    imshow("Final Blurred Image", finalBlurredImage);

    // namedWindow("Canny Edges", WINDOW_AUTOSIZE);
    // imshow("Canny Edges", edgeImage);

    namedWindow("Detected Circles", WINDOW_AUTOSIZE);
    imshow("Detected Circles", circleImage);

    waitKey(0);
    return 0;
}
