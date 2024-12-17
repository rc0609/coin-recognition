#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <filesystem>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

// Common size for all circles and templates
const Size COMMON_SIZE(100, 100);

// Function to load preprocessed edge templates into a grouped map
map<string, vector<Mat>> loadEdgeTemplates(const string& folder) {
    map<string, vector<Mat>> templates;

    for (const auto& entry : fs::directory_iterator(folder)) {
        string filename = entry.path().filename().string();
        string type;

        // Group templates based on filenames
        if (filename.find("Dime") != string::npos) type = "Dime";
        else if (filename.find("Quarter") != string::npos) type = "Quarter";
        else if (filename.find("Nickle") != string::npos || filename.find("Nickel") != string::npos) type = "Nickel";
        else if (filename.find("penny") != string::npos || filename.find("Penn") != string::npos) type = "Penny";
        else if (filename.find("one dollar") != string::npos) type = "One Dollar";
        else continue;

        Mat temp = imread(entry.path(), IMREAD_GRAYSCALE);
        if (!temp.empty()) {
            resize(temp, temp, COMMON_SIZE); // Resize to common size
            templates[type].push_back(temp);
        }
    }
    return templates;
}

// Function to calculate pixel-by-pixel edge match percentage
double calculateEdgeMatch(const Mat& edges1, const Mat& edges2) {
    int totalPixels = edges1.rows * edges1.cols;
    int matchCount = 0;

    for (int y = 0; y < edges1.rows; y++) {
        for (int x = 0; x < edges1.cols; x++) {
            if (edges1.at<uchar>(y, x) > 0 && edges2.at<uchar>(y, x) > 0) {
                matchCount++;
            }
        }
    }

    return (double(matchCount) / totalPixels) * 100.0;
}

// Function to classify a coin using edge match
pair<string, double> classifyCoin(const Mat& coinEdges, const map<string, vector<Mat>>& templates) {
    double bestMatch = 0.0;
    string bestMatchType = "Unknown";

    for (const auto& [type, images] : templates) {
        for (const auto& tmpl : images) {
            double match = calculateEdgeMatch(coinEdges, tmpl);
            if (match > bestMatch) {
                bestMatch = match;
                bestMatchType = type;
            }
        }
    }

    return {bestMatchType, bestMatch};
}

int main() {
    // Paths
    string image_path = "Final.png";                   // Input test image
    string template_folder = "image_template_edges/"; // Preprocessed edge templates folder

    Mat img = imread(image_path);
    if (img.empty()) {
        cerr << "Error: Could not load the image!" << endl;
        return -1;
    }

    // Preprocess the test image
    Mat gray, blurred;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(9, 9), 2, 2);

    // Detect circles using Hough Transform
    vector<Vec3f> circles;
    HoughCircles(blurred, circles, HOUGH_GRADIENT, 1, gray.rows / 12, 200, 50, 30, 150);

    // Load preprocessed edge templates
    map<string, vector<Mat>> templates = loadEdgeTemplates(template_folder);

    // Classify and draw detected circles
    for (size_t i = 0; i < circles.size(); i++) {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // Extract ROI around the detected coin
        Rect coinRect(center.x - radius, center.y - radius, 2 * radius, 2 * radius);
        if (coinRect.x >= 0 && coinRect.y >= 0 && coinRect.x + coinRect.width <= img.cols && coinRect.y + coinRect.height <= img.rows) {
            Mat coinROI = gray(coinRect);

            // Resize coin ROI to common size
            Mat coinResized, coinEdges;
            resize(coinROI, coinResized, COMMON_SIZE);

            // Apply Canny Edge Detection
            Canny(coinResized, coinEdges, 50, 150);

            // Save the edge-detected coin for debugging
            imwrite("test_coin_edge_" + to_string(i) + ".png", coinEdges);

            // Classify the coin
            auto [coinType, matchPercent] = classifyCoin(coinEdges, templates);

            // Draw circle and label
            circle(img, center, radius, Scalar(0, 0, 255), 2);
            string label = coinType + " (" + to_string(int(matchPercent)) + "%)";
            putText(img, label, Point(center.x - 40, center.y - radius - 10),
                    FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
        }
    }

    // Display and save the result
    imshow("Detected and Classified Coins", img);
    imwrite("output_classified.png", img);
    waitKey(0);

    return 0;
}

