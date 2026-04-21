#include "zoom.h"
#include <cmath>

cv::Mat zoomNearestNeighbor(const cv::Mat& src, const double S) {
    const int rows = src.rows * S;
    const int cols = src.cols * S;
    cv::Mat dst(rows, cols, src.type());
    dst.setTo(0);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int srcR = static_cast<int>(r / S);
            int srcC = static_cast<int>(c / S);
            srcR = std::min(srcR, src.rows - 1);
            srcC = std::min(srcC, src.cols - 1);
            dst.at<uchar>(r, c) = src.at<uchar>(srcR, srcC);
        }
    }
    return dst;
}

cv::Mat zoomBilinear(const cv::Mat& src, const double S) {
    const int rows = src.rows * S;
    const int cols = src.cols * S;
    cv::Mat dst(rows, cols, src.type());
    dst.setTo(0);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double srcR = static_cast<double>(r) / S;
            double srcC = static_cast<double>(c) / S;
            int r1 = static_cast<int>(srcR);
            int c1 = static_cast<int>(srcC);
            int r2 = std::min(r1 + 1, src.rows - 1);
            int c2 = std::min(c1 + 1, src.cols - 1);
            r1 = std::min(r1, src.rows - 1);
            c1 = std::min(c1, src.cols - 1);

            double dx = srcR - r1;   // fractional part
            double dy = srcC - c1;

            double P11 = src.at<uchar>(r1, c1);
            double P21 = src.at<uchar>(r1, c2);
            double P12 = src.at<uchar>(r2, c1);
            double P22 = src.at<uchar>(r2, c2);

            double val = (1-dx)*(1-dy)*P11
                       + dx   *(1-dy)*P21
                       + (1-dx)*dy   *P12
                       + dx   *dy    *P22;
            dst.at<uchar>(r, c) = static_cast<uchar>(std::round(val));
        }
    }

    return dst;
}

double getKernel(const double d) {
    const double a = -0.5;
    double abs_d = fabs(d);
    if (abs_d <= 1.0) {
        return (a+2) * abs_d * abs_d * abs_d - (a + 3) * abs_d * abs_d +1;
    }
    if (1.0 < abs_d   && abs_d <= 2.0) {
        return a * abs_d * abs_d * abs_d - 5 * a * abs_d * abs_d + 8*a*abs_d - 4*a;
    }
    return 0;
}

cv::Mat zoomBicubic(const cv::Mat& src, const double S) {
    const int rows = src.rows * S;
    const int cols = src.cols * S;
    cv::Mat dst(rows, cols, src.type());
    dst.setTo(0);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double srcR = static_cast<double>(r) / S;
            double srcC = static_cast<double>(c) / S;
            int r0 = floor(srcR);
            int c0 = floor(srcC);
            double newColor = 0.0;
            double wr[4], wc[4];
            //  calculate weights here instead of the inner loops
            for (int j = -1; j <= 2; j++) wr[j+1] = getKernel(srcR - (r0+j));
            for (int i = -1; i <= 2; i++) wc[i+1] = getKernel(srcC - (c0+i));

            //  sum the 4x4 grid
            for (int j = -1; j <= 2; j++) {
                for (int i = -1; i <= 2; i++) {
                    //  ri and ci are needed only for safe memory access
                    int ri = std::clamp(r0 + j, 0, src.rows - 1);
                    int ci = std::clamp(c0 + i, 0, src.cols - 1);
                    uchar srcColor = src.at<uchar>(ri, ci);
                    //  compute actual distance
                    newColor += wr[j+1]*wc[i+1]*srcColor;
                }
            }
            newColor = std::clamp(newColor, 0.0, 255.0);
            dst.at<uchar>(r, c) = static_cast<uchar>(std::round(newColor));
        }
    }
    return dst;
}