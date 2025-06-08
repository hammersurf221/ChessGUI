#include "chessboard_detector.h"

#include <QDebug>
#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>

// The previous implementation scanned contours and scored them based on Hough
// line counts.  It worked reasonably well but often failed on noisy captures.
// The new approach detects prominent horizontal and vertical lines first and
// derives the board rectangle directly from their intersections.

QRect detectChessboard(const QImage& qimage) {
    double dpr = qimage.devicePixelRatio();

    qDebug() << "[detectChessboard] image size:" << qimage.size() << "dpr" << dpr;

    cv::Mat mat(qimage.height(), qimage.width(), CV_8UC4,
                const_cast<uchar*>(qimage.bits()), qimage.bytesPerLine());

    cv::Mat gray;
    cv::cvtColor(mat, gray, cv::COLOR_BGRA2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);

    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150, 3);

    std::vector<cv::Vec2f> lines;
    cv::HoughLines(edges, lines, 1, CV_PI / 180, 120);

    std::vector<int> verticalX;
    std::vector<int> horizontalY;

    const double angleThresh = CV_PI / 12;  // 15 degrees
    for (const auto& l : lines) {
        float rho = l[0];
        float theta = l[1];
        if (theta < angleThresh || theta > CV_PI - angleThresh) {
            // near vertical
            double x = rho / std::cos(theta);
            verticalX.push_back(static_cast<int>(std::round(x)));
        } else if (std::abs(theta - CV_PI / 2) < angleThresh) {
            // near horizontal
            double y = rho / std::sin(theta);
            horizontalY.push_back(static_cast<int>(std::round(y)));
        }
    }

    if (verticalX.size() < 2 || horizontalY.size() < 2) {
        qDebug() << "[detectChessboard] insufficient lines" << verticalX.size()
                 << horizontalY.size();
        return QRect();
    }

    int minX = *std::min_element(verticalX.begin(), verticalX.end());
    int maxX = *std::max_element(verticalX.begin(), verticalX.end());
    int minY = *std::min_element(horizontalY.begin(), horizontalY.end());
    int maxY = *std::max_element(horizontalY.begin(), horizontalY.end());

    if (maxX <= minX || maxY <= minY) {
        qDebug() << "[detectChessboard] invalid rectangle from lines";
        return QRect();
    }

    cv::Rect rect(minX, minY, maxX - minX, maxY - minY);
    qDebug() << "[detectChessboard] rect" << rect.x << rect.y << rect.width
             << rect.height;

    return QRect(rect.x / dpr, rect.y / dpr, rect.width / dpr,
                 rect.height / dpr);
}
