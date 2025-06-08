#include "chessboard_detector.h"
#include <opencv2/opencv.hpp>

QRect detectChessboard(const QImage& qimage) {
    cv::Mat mat(qimage.height(), qimage.width(), CV_8UC4,
                const_cast<uchar*>(qimage.bits()), qimage.bytesPerLine());

    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_BGRA2BGR);

    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    cv::Rect bestRect;

    for (const auto& contour : contours) {
        double peri = cv::arcLength(contour, true);
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 0.02 * peri, true);

        if (approx.size() == 4) {
            cv::Rect rect = cv::boundingRect(approx);
            double aspectRatio = static_cast<double>(rect.width) / rect.height;
            double area = rect.area();

            if (aspectRatio > 0.8 && aspectRatio < 1.2 && area > maxArea) {
                maxArea = area;
                bestRect = rect;
            }
        }
    }

    if (maxArea == 0)
        return QRect(); // no valid board found

    return QRect(bestRect.x, bestRect.y, bestRect.width, bestRect.height);
}
