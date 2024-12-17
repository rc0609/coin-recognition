#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

int main() {
    // Input and Output Paths
    string input_folder = "image_template/";       // Folder containing the template images
    string output_folder = "image_template_edges/"; // Folder to save the edge images

    // Check if output folder exists; if not, create it
    if (!fs::exists(output_folder)) {
        fs::create_directory(output_folder);
    }

    // Process all images in the input folder
    for (const auto& entry : fs::directory_iterator(input_folder)) {
        string input_path = entry.path().string();
        string filename = entry.path().filename().string();
        string output_path = output_folder + "edge_" + filename; // Add 'edge_' prefix for output

        // Load the template image
        Mat img = imread(input_path, IMREAD_GRAYSCALE);
        if (img.empty()) {
            cerr << "Error: Could not load image " << input_path << endl;
            continue;
        }

        // Denoise the image
        Mat denoised;
        fastNlMeansDenoising(img, denoised, 30, 7, 21); // Non-local means denoising

        // Apply GaussianBlur to smooth further
        Mat blurred;
        GaussianBlur(denoised, blurred, Size(5, 5), 1.5);

        // Apply Canny Edge Detection
        Mat edges;
        Canny(blurred, edges, 50, 100); // Canny thresholds: 50 and 150

        // Save the resulting edge image
        imwrite(output_path, edges);

        cout << "Processed and saved: " << output_path << endl;
    }

    cout << "All template images converted to clean Canny edges and saved in: " << output_folder << endl;
    return 0;
}
