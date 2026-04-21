#include "shrink.h"

cv::Mat shrinkSubsample(const cv::Mat& src, const double S) {
    const int rows = src.rows * S;
    const int cols = src.cols * S;
    auto scaled = cv::Mat(rows, cols, src.type());
    scaled.setTo(0);    //  black image

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int oldR = static_cast<int>(r / S);
            int oldC = static_cast<int>(c / S);
            oldR = std::min(oldR, src.rows - 1);
            oldC = std::min(oldC, src.cols - 1);
            scaled.at<uchar>(r, c) = src.at<uchar>(oldR, oldC);
        }
    }

    return scaled;
}

cv::Mat shrinkBoxAverage(const cv::Mat& src, const double S) {
    const int rows = src.rows * S;
    const int cols = src.cols * S;
    auto scaled = cv::Mat(rows, cols, src.type());
    scaled.setTo(0);
    const int blockSize = static_cast<int>(1.0 / S);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            //  for each new pixel average a block of pixels from the source
            int newColor = 0;
            int pixelCount = 0;
            for (int i = r * blockSize; i < r * blockSize + blockSize; ++i) {
                for (int j = c * blockSize; j < c * blockSize + blockSize; ++j) {
                    if (i < src.rows && j < src.cols) {
                        newColor += src.at<uchar>(i, j);
                        pixelCount++;
                    }
                }
            }
            if (pixelCount > 0) {
                scaled.at<uchar>(r, c) = static_cast<uchar>(newColor / pixelCount);
            }
        }
    }
    return scaled;
}