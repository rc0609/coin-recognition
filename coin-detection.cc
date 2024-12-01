#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <cmath>

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

//Test each kernel size by calculating PSNR and use the kernel size with best PSNR
int bestKernelSize = 3;
double bestPSNR = 0;

//Try kernel sizes 3X3, 5X5, 7X7, 11X11
for(int i = 3; i <= 11; i += 2){
    Mat gaussianBlurredImage;
    GaussianBlur(grayImage, gaussianBlurredImage, Size(i, i), 1, 1); //Sigma = 1

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

//Display the original, grayscale, and edge-detected images
imshow("Original Image", image);
imshow("Grayscale Image", grayImage);
imshow("Final Blurred Image", finalBlurredImage);
imshow("Canny Edges", edgeImage);

waitKey(0);
return 0;

}
