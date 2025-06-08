#include "chessboard_detector.h"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <cmath>
#include <QDebug>

static int lineScore(const cv::Mat& roi) {
    cv::Mat edges;
    cv::Canny(roi, edges, 50, 150);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180, 30, roi.cols / 8, 10);

    int score = 0;
    for (const auto& l : lines) {
        double angle = std::atan2(l[3] - l[1], l[2] - l[0]) * 180.0 / CV_PI;
        angle = std::abs(angle);
        if (angle < 10 || std::abs(angle - 90) < 10)
            score++;
    }
    return score;
}

QRect detectChessboard(const QImage& qimage) {
    double dpr = qimage.devicePixelRatio();

    qDebug() << "[detectChessboard] image size:" << qimage.size() << "dpr" << dpr;

    cv::Mat mat(qimage.height(), qimage.width(), CV_8UC4,
                const_cast<uchar*>(qimage.bits()), qimage.bytesPerLine());

    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_BGRA2BGR);

    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

    cv::Mat thresh;
    cv::adaptiveThreshold(gray, thresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY, 11, 2);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat closed;
    cv::morphologyEx(thresh, closed, cv::MORPH_CLOSE, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(closed, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    qDebug() << "[detectChessboard] contours found:" << contours.size();

    double imgArea = static_cast<double>(closed.rows * closed.cols);

    struct Candidate {
        cv::Rect rect;
        int score;
    };

    std::vector<Candidate> candidates;

    for (const auto& contour : contours) {
        double peri = cv::arcLength(contour, true);
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 0.02 * peri, true);

        if (approx.size() == 4) {
            cv::Rect rect = cv::boundingRect(approx);
            double aspect = static_cast<double>(rect.width) / rect.height;
            double area = rect.area();

            if (aspect > 0.75 && aspect < 1.25 && area > imgArea * 0.01) {
                cv::Rect roiRect = rect & cv::Rect(0, 0, gray.cols, gray.rows);
                int score = lineScore(gray(roiRect));
                candidates.push_back({rect, score});
                qDebug() << "[detectChessboard] candidate" << rect.x << rect.y
                         << rect.width << rect.height << "score" << score;
            }
        }
    }

    if (candidates.empty()) {
        qDebug() << "[detectChessboard] no candidate rectangles";
        return QRect();
    }

    auto best = std::max_element(candidates.begin(), candidates.end(),
                                 [](const Candidate& a, const Candidate& b) {
                                     if (a.score == b.score)
                                         return a.rect.area() < b.rect.area();
                                     return a.score < b.score;
                                 });

    cv::Rect bestRect = best->rect;
    qDebug() << "[detectChessboard] best rect" << bestRect.x << bestRect.y
             << bestRect.width << bestRect.height << "score" << best->score;

    return QRect(bestRect.x / dpr, bestRect.y / dpr,
                 bestRect.width / dpr, bestRect.height / dpr);
}
