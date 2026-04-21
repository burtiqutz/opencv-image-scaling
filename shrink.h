#pragma once
#include <opencv2/opencv.hpp>

cv::Mat shrinkSubsample(const cv::Mat& src, double S);   // S < 1
cv::Mat shrinkBoxAverage(const cv::Mat& src, double S);  // S < 1
