#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;

float getCircularityThresh(vector<Point> cntr);

int main()
{
    Mat img, gray;
    img = imread("small.png"); // Replace with your image path

    if (img.empty()) {
        cerr << "Error: Could not load the image!" << endl;
        return -1;
    }

    cvtColor(img, gray, COLOR_BGR2GRAY);

    // Smooth it to reduce noise
    GaussianBlur(gray, gray, Size(3, 3), 1.5, 1.5);
    imshow("Blurred", gray);

    // Otsu threshold
    Mat opimg = Mat(gray.rows, gray.cols, CV_8UC1);
    double otsu_thresh_val = threshold(gray, opimg, 0, 255, THRESH_BINARY | THRESH_OTSU);
    imshow("Otsu", opimg);

    double high_thresh_val = otsu_thresh_val;
    double lower_thresh_val = otsu_thresh_val * 0.5;

    // Canny edge detection
    Mat edges;
    Canny(gray, edges, lower_thresh_val, high_thresh_val, 3);
    imshow("Canny", edges);

    // Morphology (Dilation)
    int dilation_type = MORPH_RECT, dilation_size = 1;
    Mat element = getStructuringElement(dilation_type,
                                        Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                                        Point(dilation_size, dilation_size));
    morphologyEx(edges, edges, MORPH_DILATE, element);
    Canny(edges, edges, 50, 200, 3);
    imshow("Canny after Morphology", edges);

    // Contour analysis
    vector<vector<Point>> contours, contoursfil;
    vector<Vec4i> hierarchy;
    Mat contourImg2 = Mat::ones(edges.rows, edges.cols, edges.type());

    findContours(edges.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

    for (size_t j = 0; j < contours.size(); j++) {
        if ((hierarchy[j][2] >= 0) && (hierarchy[j][3] < 0)) {
            float circThresh = getCircularityThresh(contours[j]);
            if ((circThresh > 10) && (circThresh <= 30)) {
                contoursfil.push_back(contours[j]);
            }
        }
    }

    for (size_t j = 0; j < contoursfil.size(); j++) {
        drawContours(contourImg2, contoursfil, (int)j, Scalar(255, 255, 255), 1, 8);
    }
    imshow("Filtered Contours", contourImg2);

    // Hough Circle Transform
    vector<Vec3f> circles;
    HoughCircles(contourImg2, circles, HOUGH_GRADIENT, 2, gray.rows / 8, 200, 30, 15, 45);

    sort(circles.rbegin(), circles.rend(), [](const Vec3f &left, const Vec3f &right) {
        return left[2] < right[2];
    });

    float largestRadius = circles[0][2];
    float change = 0.0f;

    for (size_t i = 0; i < circles.size(); i++) {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        float radius = circles[i][2];

        circle(img, center, 3, Scalar(0, 255, 0), -1);
        circle(img, center, radius, Scalar(0, 0, 255), 2);
        rectangle(img, Point(center.x - radius - 5, center.y - radius - 5),
                  Point(center.x + radius + 5, center.y + radius + 5),
                  Scalar(0, 0, 255), 1);

        float ratio = ((radius * radius) / (largestRadius * largestRadius));

        if (ratio >= 0.85) {
            putText(img, "Quarter", Point(center.x - radius, center.y + radius + 15),
                    FONT_HERSHEY_COMPLEX_SMALL, 0.7, Scalar(0, 255, 255), 1);
            change += 0.25;
        } else if ((ratio >= 0.68) && (ratio < 0.85)) {
            putText(img, "5-cents", Point(center.x - radius, center.y + radius + 15),
                    FONT_HERSHEY_COMPLEX_SMALL, 0.7, Scalar(0, 255, 255), 1);
            change += 0.05;
        } else if ((ratio >= 0.60) && (ratio < 0.68)) {
            putText(img, "1-cent", Point(center.x - radius, center.y + radius + 15),
                    FONT_HERSHEY_COMPLEX_SMALL, 0.7, Scalar(0, 255, 255), 1);
            change += 0.01;
        } else if ((ratio >= 0.40) && (ratio < 0.60)) {
            putText(img, "Dime", Point(center.x - radius, center.y + radius + 15),
                    FONT_HERSHEY_COMPLEX_SMALL, 0.7, Scalar(0, 255, 255), 1);
            change += 0.10;
        }
    }

    putText(img, "Total Money: $" + to_string(change), Point(img.cols / 10, img.rows - img.rows / 10),
            FONT_HERSHEY_COMPLEX_SMALL, 0.7, Scalar(0, 255, 255), 1);

    imshow("Circles", img);
    waitKey(0);
    return 0;
}

float getCircularityThresh(vector<Point> cntr) {
    float perm = arcLength(Mat(cntr), true);
    float area = contourArea(Mat(cntr));
    return (perm * perm) / area;
}


