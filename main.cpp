#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include "zoom.h"
#include "shrink.h"

int main() {
    cv::Mat src = cv::imread("test2.bmp", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cout << "Could not open or find the image 'test.bmp'" << std::endl;
        return -1;
    }

    cv::Mat decimated = shrinkSubsample(src, 0.5);
    cv::Mat boxAverage = shrinkBoxAverage(src, 0.5);
    cv::Mat nearestNeighbor = zoomNearestNeighbor(src, 2);
    cv::Mat biInterpolation = zoomBilinear(src, 2);
    cv::Mat cubicInterp = zoomBicubic(src, 2);
    cv::imwrite("output_decimated.bmp", decimated);
    cv::imwrite("output_box.bmp", boxAverage);
    cv::imwrite("output_nearestNeighbor.bmp", nearestNeighbor);
    cv::imwrite("output_bilinear.bmp", biInterpolation);
    cv::imwrite("output_cubic.bmp", cubicInterp);

    /*
    //  OpenCV implementations for comparision
    cv::resize(src, dst, {}, S, S, cv::INTER_NEAREST);   // nearest neighbor
    cv::resize(src, dst, {}, S, S, cv::INTER_LINEAR);    // bilinear
    cv::resize(src, dst, {}, S, S, cv::INTER_CUBIC);     // bicubic
    cv::resize(src, dst, {}, S, S, cv::INTER_AREA);      // box average (shrink)
    */

    return 0;
}