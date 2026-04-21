#pragma once
#include <opencv2/opencv.hpp>

cv::Mat zoomNearestNeighbor(const cv::Mat& src, double S);
cv::Mat zoomBilinear(const cv::Mat& src, double S);
cv::Mat zoomBicubic(const cv::Mat& src, double S);