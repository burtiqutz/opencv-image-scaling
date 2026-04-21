#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include "zoom.h"
#include "shrink.h"

using namespace std::chrono;

cv::Mat timeIt(const std::string& name, std::function<cv::Mat()> fn) {
    auto t0 = high_resolution_clock::now();
    cv::Mat result = fn();
    auto t1 = high_resolution_clock::now();
    double ms = duration<double, std::milli>(t1 - t0).count();
    std::cout << name << ": " << ms << " ms\n";
    return result;
}

double comparePSNR(const cv::Mat& a, const cv::Mat& b) {
    cv::Mat diff;
    cv::absdiff(a, b, diff);
    diff.convertTo(diff, CV_32F);
    diff = diff.mul(diff);
    double mse = cv::mean(diff)[0];
    if (mse == 0) return 100.0;
    return 10.0 * log10(255.0 * 255.0 / mse);
}

std::pair<cv::Mat, cv::Mat> cropToSmaller(const cv::Mat& a, const cv::Mat& b) {
    int rows = std::min(a.rows, b.rows);
    int cols = std::min(a.cols, b.cols);
    return { a(cv::Rect(0, 0, cols, rows)), b(cv::Rect(0, 0, cols, rows)) };
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <scale_factor>\n";
        std::cout << "  scale_factor > 1 for zoom, < 1 for shrink\n";
        return -1;
    }

    double S = std::stod(argv[1]);

    cv::Mat src = cv::imread("test.bmp", cv::IMREAD_GRAYSCALE);
    if (src.empty()) {
        std::cout << "Could not open or find the image 'test.bmp'\n";
        return -1;
    }

    std::cout << "Source: " << src.cols << "x" << src.rows << "  S=" << S << "\n\n";

    if (S > 1.0) {
        std::cout << "=== ZOOM ===\n";
        auto myNearest  = timeIt("My  nearest neighbor", [&]{ return zoomNearestNeighbor(src, S); });
        auto myBilinear = timeIt("My  bilinear        ", [&]{ return zoomBilinear(src, S); });
        auto myBicubic  = timeIt("My  bicubic         ", [&]{ return zoomBicubic(src, S); });

        cv::Mat cvNearest, cvBilinear, cvBicubic;
        timeIt("CV  nearest neighbor", [&]{ cv::resize(src, cvNearest,  {}, S, S, cv::INTER_NEAREST); return cvNearest; });
        timeIt("CV  bilinear        ", [&]{ cv::resize(src, cvBilinear, {}, S, S, cv::INTER_LINEAR);  return cvBilinear; });
        timeIt("CV  bicubic         ", [&]{ cv::resize(src, cvBicubic,  {}, S, S, cv::INTER_CUBIC);   return cvBicubic; });

        std::cout << "\n=== PSNR vs OpenCV (higher = closer to CV) ===\n";
        auto [n1, n2] = cropToSmaller(myNearest,  cvNearest);
        auto [b1, b2] = cropToSmaller(myBilinear, cvBilinear);
        auto [c1, c2] = cropToSmaller(myBicubic,  cvBicubic);
        std::cout << "Nearest:  " << comparePSNR(n1, n2) << " dB\n";
        std::cout << "Bilinear: " << comparePSNR(b1, b2) << " dB\n";
        std::cout << "Bicubic:  " << comparePSNR(c1, c2) << " dB\n";

        std::cout << "\n=== Round-trip PSNR (zoom in then shrink back, higher = better) ===\n";
        auto rtNearest  = shrinkSubsample(myNearest,   1.0 / S);
        auto rtBilinear = shrinkBoxAverage(myBilinear, 1.0 / S);
        auto rtBicubic  = shrinkBoxAverage(myBicubic,  1.0 / S);
        auto [rn1, rn2] = cropToSmaller(rtNearest,  src);
        auto [rb1, rb2] = cropToSmaller(rtBilinear, src);
        auto [rc1, rc2] = cropToSmaller(rtBicubic,  src);
        std::cout << "Nearest:  " << comparePSNR(rn1, rn2) << " dB\n";
        std::cout << "Bilinear: " << comparePSNR(rb1, rb2) << " dB\n";
        std::cout << "Bicubic:  " << comparePSNR(rc1, rc2) << " dB\n";

        cv::imwrite("out_nearest.bmp",     myNearest);
        cv::imwrite("out_bilinear.bmp",    myBilinear);
        cv::imwrite("out_bicubic.bmp",     myBicubic);
        cv::imwrite("out_nearest_cv.bmp",  cvNearest);
        cv::imwrite("out_bilinear_cv.bmp", cvBilinear);
        cv::imwrite("out_bicubic_cv.bmp",  cvBicubic);

    } else if (S < 1.0) {
        std::cout << "=== SHRINK ===\n";
        auto mySubsample = timeIt("My  subsample  ", [&]{ return shrinkSubsample(src, S); });
        auto myBox       = timeIt("My  box average", [&]{ return shrinkBoxAverage(src, S); });

        cv::Mat cvNearest, cvArea;
        timeIt("CV  nearest (subsample equiv)", [&]{ cv::resize(src, cvNearest, {}, S, S, cv::INTER_NEAREST); return cvNearest; });
        timeIt("CV  area    (box equiv)      ", [&]{ cv::resize(src, cvArea,    {}, S, S, cv::INTER_AREA);    return cvArea; });

        std::cout << "\n=== PSNR vs OpenCV (higher = closer to CV) ===\n";
        auto [s1, s2] = cropToSmaller(mySubsample, cvNearest);
        auto [bx1, bx2] = cropToSmaller(myBox, cvArea);
        std::cout << "Subsample:   " << comparePSNR(s1, s2)   << " dB\n";
        std::cout << "Box average: " << comparePSNR(bx1, bx2) << " dB\n";

        std::cout << "\n=== Round-trip PSNR (shrink then zoom back, higher = better) ===\n";
        auto rtSubsample = zoomNearestNeighbor(mySubsample, 1.0 / S);
        auto rtBox       = zoomBilinear(myBox,              1.0 / S);
        auto [rs1, rs2] = cropToSmaller(rtSubsample, src);
        auto [rb1, rb2] = cropToSmaller(rtBox,        src);
        std::cout << "Subsample:   " << comparePSNR(rs1, rs2) << " dB\n";
        std::cout << "Box average: " << comparePSNR(rb1, rb2) << " dB\n";

        cv::imwrite("out_subsample.bmp",    mySubsample);
        cv::imwrite("out_box.bmp",          myBox);
        cv::imwrite("out_subsample_cv.bmp", cvNearest);
        cv::imwrite("out_box_cv.bmp",       cvArea);

    } else {
        std::cout << "Scale factor must be != 1.0\n";
        return -1;
    }

    return 0;
}