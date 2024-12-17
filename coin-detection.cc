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
    string imagePath = "C:/Users/rdire/Documents/coin-recognition/img/test_img/multiple-coin.jpg"; //Include your image path
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

    HoughCircles(finalBlurredImage, circles, HOUGH_GRADIENT, 1, finalBlurredImage.rows/8, 250, 30, 0, 0);

    int penny = 0;
    int dime = 0;
    int nickel = 0; 
    int quarter = 0;
    vector<double> radius(circles.size());
    vector<double> relative_radius(circles.size());
    double max_radius = 0;

    Mat circleImage = image.clone();
    for(size_t i = 0; i < circles.size(); i++){
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        radius[i] = cvRound(circles[i][2]); //Store the radius of detected circle 
        if(radius[i] > max_radius){
            max_radius = radius[i];
        }
    }

    for(size_t i = 0; i < circles.size(); i++){
        cout << "max_radius inside second loop: " << max_radius << endl;
        relative_radius[i] = radius[i] / max_radius;
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));

        //Draw circles detected
        circle(circleImage, center, radius[i], Scalar(0, 255, 0), 3, 8, 0); //Green border for detected coins

        string coinLabel;
        if (relative_radius[i] < 0.76) { 
            coinLabel = "Dime";
            dime++;
        } else if (relative_radius[i] < 0.85) {
            coinLabel = "Penny";
            penny++;
        } else if (relative_radius[i] < 0.94) {
            coinLabel = "Nickel";
            nickel++;
        } else if (relative_radius[i] < 1.01) {
            coinLabel = "Quarter";
            quarter++;
        }

        //Label each coin
        putText(circleImage, coinLabel, center - Point(0, radius[i] + 10), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 1, 8);
    }

    int total_coins = dime + nickel + quarter + penny;
    double currency = (quarter * 25.0 + dime * 10.0 + nickel * 5.0 + penny * 0.1)/100.0;

    //Prepare text to display
    string resultText = "Total Coins: " + to_string(total_coins) + " | ";
    resultText += "Penny: " + to_string(penny) + " | ";
    resultText += "Dime: " + to_string(dime) + " | ";
    resultText += "Nickel: " + to_string(nickel) + " | ";
    resultText += "Quarter: " + to_string(quarter) + " | ";
    resultText += "Currency: $" + to_string(currency);

    int fontFace = FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.8; 
    int thickness = 1;
    Scalar textColor(0, 0, 0); //Black color for text
    Scalar bgColor(255, 255, 255); //White background

    int offsetX = 20;  
    int yPosition = image.rows - 10; //Position for the text along the bottom edge

    Size textSize = getTextSize(resultText, fontFace, fontScale, thickness, nullptr);
    int textWidth = textSize.width;
    int textHeight = textSize.height;

    //Put white background behind the text
    rectangle(circleImage, 
              Point(image.cols - textWidth - offsetX - 10, yPosition - textHeight - 5), //Top-left corner of rectangle
              Point(image.cols - offsetX, yPosition + 5), //Bottom-right corner of rectangle
              bgColor, -1); 

    putText(circleImage, resultText, 
            Point(image.cols - textWidth - offsetX, yPosition), 
            fontFace, fontScale, textColor, thickness);

    // cout << "Total Coins: " << total_coins << endl;
    // cout << "Penny: " << penny << ", Dime: " << dime << ", Nickel: " << nickel << ", Quarter: " << quarter << endl;
    // cout << "Currency: $" << currency << endl;

    namedWindow("Detected Circles", WINDOW_AUTOSIZE);
    imshow("Detected Circles", circleImage);
    // imwrite("C:/Users/rdire/Documents/coin-recognition/img/result.jpg", circleImage);

    waitKey(0);
    return 0;
}
