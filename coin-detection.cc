#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

//Function to match the ORB (Oriented FAST and Rotated BRIEF) features between the test image and coin template
int matchFeatures(Mat &test_image, Mat &template_image, Ptr<ORB> orb){
    //Convert the template image to grayscale
    Mat gray_template;
    cvtColor(template_image, gray_template, COLOR_BGR2RGB);

    //Detect ORB keypoints and descriptors
    vector<KeyPoint> keypoints_test, keypoints_template;
    Mat descriptors_test, descriptors_template;
    orb->detectAndCompute(test_image, noArray(), keypoints_test, descriptors_test);
    orb->detectAndCompute(gray_template, noArray(), keypoints_template, descriptors_template);

    //Use BruteForceMatcher to match descriptors
    BFMatcher matcher(NORM_HAMMING, true); //true for cross-checking
    vector<DMatch> matches;
    matcher.match(descriptors_test, descriptors_template, matches);

    return matches.size();  //Return the number of matches

}

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

    //Load coin templates both front and back side of coins
    vector<Mat> coin_templates;
    vector<string> coin_labels = {"Penny", "Penny Reverse", "Nickel", "Nickel Reverse", "Dime", "Dime Reverse", "Quarter", "Quarter Reverse"};
    //Include your image path
    coin_templates.push_back(imread("C:/Users/rdire/Documents/coin-recognition/img/template/penny.jpg"));
    coin_templates.push_back(imread("C:/Users/rdire/Documents/coin-recognition/img/template/penny_rev.jpg"));
    coin_templates.push_back(imread("C:/Users/rdire/Documents/coin-recognition/img/template/nickel.jpg"));
    coin_templates.push_back(imread("C:/Users/rdire/Documents/coin-recognition/img/template/nickel_rev.jpg"));
    coin_templates.push_back(imread("C:/Users/rdire/Documents/coin-recognition/img/template/dime.jpg"));
    coin_templates.push_back(imread("C:/Users/rdire/Documents/coin-recognition/img/template/dime_rev.jpg"));
    coin_templates.push_back(imread("C:/Users/rdire/Documents/coin-recognition/img/template/quarter.jpg"));
    coin_templates.push_back(imread("C:/Users/rdire/Documents/coin-recognition/img/template/quarter_rev.jpg"));

    //Initialize ORB detector
    Ptr<ORB> orb = ORB::create();

    Mat circleImage = image.clone();
    for (size_t i = 0; i < circles.size(); i++) {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        //Crop the detected coin from the image
        Rect roi(center.x - radius, center.y - radius, 2 * radius, 2 * radius);
        Mat coin = image(roi);

        //Match this coin against all templates
        int best_match_count = 0;
        string best_coin_label;
        for (size_t j = 0; j < coin_templates.size(); j++) {
            int match_count = matchFeatures(coin, coin_templates[j], orb);
            if (match_count > best_match_count) {
                best_match_count = match_count;
                best_coin_label = coin_labels[j];
            }
        }

        //Draw the circle and label the coin type
        circle(image, center, radius, Scalar(0, 255, 0), 3);
        putText(image, best_coin_label, Point(center.x - radius, center.y - radius - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
    }

    //Display the result
    namedWindow("Detected Coins", WINDOW_AUTOSIZE);
    imshow("Detected Coins", image);
    waitKey(0);

    return 0;
}
