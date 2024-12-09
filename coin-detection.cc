#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

//Function to calculate PSNR
double calculatePSNR(const Mat& original, const Mat& processed){
    //Enusre that inputs have same size and type
    assert(original.size() == processed.size() && original.type() == processed.type());
    Mat difference;
    //Compute absolute difference of original and processed images
    absdiff(original, processed, difference);
    //Convert difference to float for calculation precision
    difference.convertTo(difference, CV_32F);
    //Square the difference by Element-wise multiplication
    difference = difference.mul(difference); 

    //Sum up the squared difference
    Scalar sumDifference = sum(difference);
    //Calculate Mean Square Error
    double mse = sumDifference[0] / (double)(original.total());

    if(mse == 0){ //Image is identical so set it to INFINITY
        return INFINITY; 
    }

    double max_pixel = 255.0;
    //Calculate PSNR using formula, 10 * (max_pixel^2) / mse
    double PSNR = 10.0 * log10((max_pixel * max_pixel) / mse);

    return PSNR;    
}

int main() {
    string imagePath = "C:/Users/rdire/Documents/coin-recognition/img/multiple-coin.jpg";
    Mat image = imread(imagePath, IMREAD_COLOR);

    if (image.empty()) { //Error handling
    cout << "Could not read the image: " << imagePath << endl;
    return 1;
}

//Convert image to grayscale
Mat grayImage;
cvtColor(image, grayImage, COLOR_BGR2GRAY);

//Contrast enhancement using CLAHE (Contrast Limited Adaptive Histogram Equalization)
Mat enhancedImage;
//2.0 = clip limit, (8, 8) = tile size
Ptr<CLAHE> clahe = createCLAHE(2.0, Size(8, 8));
clahe->apply(grayImage, enhancedImage);


//Test each kernel size by calculating PSNR and use the kernel size with best PSNR
int bestKernelSize = 3;
double bestPSNR = 0;

//Try kernel sizes 3X3, 5X5, 7X7, 11X11
for(int i = 3; i <= 11; i += 2){
    Mat gaussianBlurredImage;
    GaussianBlur(enhancedImage, gaussianBlurredImage, Size(i, i), 2, 2); //Sigma = 1

    //Calculate PSNR between original and blurred image
    double PSNR = calculatePSNR(grayImage, gaussianBlurredImage);
    cout << "Kernel Size: " << i << "x" << i << ", PSNR: " << PSNR << endl;

    //If best PSNR found, update the best PSNR and best kernel size variables
    if(PSNR > bestPSNR){
        bestPSNR = PSNR;
        bestKernelSize = i;
    }
}

cout << "Best Kernel size: " << bestKernelSize << endl;

//Apply Gaussian Blur to with chosen kernel size to remove possible noises
Mat finalBlurredImage;
GaussianBlur(grayImage, finalBlurredImage, Size(bestKernelSize, bestKernelSize), 1, 1); 

//Apply Canny Edge detection to extract edge map
Mat edgeImage;
//80 = lower threshold so ignore everything below, 200 = higher threshold so everything above 200 is strong edge
Canny(finalBlurredImage, edgeImage, 80, 200);

//Perform Hough Circle Transform
//Vector to store the circle, represents circle in three float values, centerX, centerY, radius
vector<Vec3f> circles;

/*
HOUGH_GRADIENT: method to detect circle
dp = 1: accumulator has same resolution as input image
minDistance = edgeImage.rows / 8: The min distance between the centers of detected circles to prevent overlapping circles from being detected
param1 = 200: The higher threshold for the Canny edge detector used internally by the function
param2 = 100: The accumulator threshold for the circle centers. A smaller value allows for detection of more circles
minRadius = 10: The minimum radius of circles to detect
maxRadius = 100: he maximum radius of circles to detect 
*/
HoughCircles(edgeImage, circles, HOUGH_GRADIENT, 1, edgeImage.rows / 8, 200, 100, 10, 100);

Mat circleImage = image.clone(); //Copy the input image
for(size_t i = 0; i < circles.size(); ++i){
    Vec3i c = circles[i]; //Get three values of circle
    Point center = Point(c[0], c[1]);
    int radius = c[2];

    //Calculate circularity
    double perimeter = 2 * CV_PI * radius;
    double area = CV_PI * radius * radius;
    double circularity = (4 * CV_PI * area) / (perimeter * perimeter);

    //Filter circles based on radius and circularity threshold
    int radius_threshold_min = 1;
    int radius_threshold_max= 2000;
    double circularity_threshold = 0.8;

    if (radius >= radius_threshold_min && radius <= radius_threshold_max && circularity > circularity_threshold){
        circle(circleImage, center, radius, Scalar(0, 255, 0), 2); //Green border for coins
        putText(circleImage, "Coin", center, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
    }
    else {
        circle(circleImage, center, radius, Scalar(0, 0, 255), 2); //Red border for non-coins
        putText(circleImage, "Non-Coin", center, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1);
    }
}

//Display the original, grayscale, and edge-detected images
namedWindow("Original Image", WINDOW_AUTOSIZE);
imshow("Original Image", image);

namedWindow("Grayscale Image", WINDOW_AUTOSIZE);
imshow("Grayscale Image", grayImage);

namedWindow("Enhanced Image", WINDOW_AUTOSIZE);
imshow("Enhanced Image", enhancedImage);

namedWindow("Final Blurred Image", WINDOW_AUTOSIZE);
imshow("Final Blurred Image", finalBlurredImage);

namedWindow("Canny Edges", WINDOW_AUTOSIZE);
imshow("Canny Edges", edgeImage);

namedWindow("Detected Circles", WINDOW_AUTOSIZE);
imshow("Detected Circles", circleImage);

waitKey(0);
return 0;

}