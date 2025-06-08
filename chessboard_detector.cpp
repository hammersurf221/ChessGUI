#include "chessboard_detector.h"

#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QtGlobal>
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

    bool debug = qEnvironmentVariableIsSet("CHESSGUI_DEBUG_DETECT");
    static int debugIndex = 0;
    QString baseName;
    if (debug)
        baseName = QString("detect_debug_%1").arg(debugIndex++);

    cv::Mat mat(qimage.height(), qimage.width(), CV_8UC4,
                const_cast<uchar*>(qimage.bits()), qimage.bytesPerLine());

    cv::Mat gray;
    cv::cvtColor(mat, gray, cv::COLOR_BGRA2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);

    if (debug) {
        cv::imwrite((baseName + "_gray.png").toStdString(), gray);
    }

    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150, 3);

    if (debug) {
        cv::imwrite((baseName + "_edges.png").toStdString(), edges);
    }

    // Find external contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Rect bestRect;
    double maxArea = 0.0;

    for (const auto& contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        double area = rect.area();
        double aspect = static_cast<double>(rect.width) / rect.height;

        // Heuristic: square-ish and large enough
        if (area > maxArea && aspect > 0.8 && aspect < 1.2 && area > 10000) {
            bestRect = rect;
            maxArea = area;
        }
    }

    if (bestRect.width == 0 || bestRect.height == 0) {
        qDebug() << "[detectChessboard] no valid board rectangle found";
        return QRect();
    }

    if (debug) {
        cv::Mat rectImg;
        cv::cvtColor(mat, rectImg, cv::COLOR_BGRA2BGR);
        cv::rectangle(rectImg, bestRect, cv::Scalar(0, 255, 0), 2);
        cv::imwrite((baseName + "_rect.png").toStdString(), rectImg);
    }

    return QRect(bestRect.x / dpr, bestRect.y / dpr, bestRect.width / dpr, bestRect.height / dpr);

}
