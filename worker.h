#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <atomic>
#include <string>
#include <sstream>
#include <map>
#include <QThread>
#include<QSharedPointer>
#include <QImage>
#include <QPixmap>
#include <QCoreApplication>
#include <opencv2/opencv.hpp>
#include "inference.h"
#include "OriginIterator.h"



class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(QObject* parent = nullptr) :QObject(parent), m_inf("C:/Users/Georg/Downloads/yolo11n.onnx", cv::Size(640, 640), "C:/Users/Georg/Downloads/classes.txt", false), keepRun(true){}
    void setData(QSharedPointer<OriginIteratorBase> frameIter) {
        m_frameIter = frameIter;
    }
    void stopWork() {
        keepRun = false;
    }
signals:
    void updateRes(QPixmap originPixmap, QPixmap resPixmap);
    void updateProgress(int progress);
    void updateRecords(QString origin, std::vector<std::map<QString, QString>> detected);
    void finished(std::string err);
    
public slots:
    void doWork() {
        while (keepRun) {
            try {
                cv::Mat frame(std::move(m_frameIter->next()));
                if (frame.empty()) {
                    break;
                }
                QPixmap originPixmap = cvMatToQPixmap(frame);
                std::vector<Detection> output = m_inf.runInference(frame);
                auto detected = drawDetections(frame, output);
                emit updateRes(originPixmap, cvMatToQPixmap(frame));
                emit updateProgress(m_frameIter->progress());
                if (!detected.empty()) {
                    emit updateRecords(m_frameIter->origin(), detected);
                }
                QCoreApplication::processEvents();
            }
            catch (const std::exception& e) {
                emit finished(e.what());
                return;
            }
        }
        emit finished("");
    }
private:
    QSharedPointer<OriginIteratorBase> m_frameIter;
    Inference m_inf;
    std::atomic<bool> keepRun;
    std::vector<std::map<QString, QString>> drawDetections(cv::Mat& frame, const std::vector<Detection>& output) {
        std::vector<std::map<QString, QString>> detected;

        for (const auto& detection : output) {
            cv::Rect box = detection.box;
            cv::Scalar color = detection.color;

            cv::rectangle(frame, box, color, 2);
            std::string confidence = std::to_string(detection.confidence).substr(0, 4);
            std::string classString = detection.className + ' ' + confidence;
            cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
            cv::Rect textBox(box.x, box.y - 40, textSize.width + 10, textSize.height + 20);

            cv::rectangle(frame, textBox, color, cv::FILLED);
            cv::putText(frame, classString, cv::Point(box.x + 5, box.y - 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
            std::map<QString, QString> res{};
            res["class"] = QString::fromStdString(detection.className);
            std::stringstream area;
            area << "xmin: " << box.x << ", ymin: " << box.y << ", xmax: " << box.x + box.width << ", ymax: " << box.y + box.height;
            res["area"] = QString::fromStdString(area.str());
            res["confidence"] = QString::fromStdString(confidence);
            detected.push_back(res);
        }
        return detected;
    }
    QPixmap cvMatToQPixmap(const cv::Mat& mat) {
        cv::Mat rgbFrame;
        if (mat.channels() == 1) {
            cv::cvtColor(mat, rgbFrame, cv::COLOR_GRAY2RGB);
        }
        else if (mat.channels() == 4) {
            cv::cvtColor(mat, rgbFrame, cv::COLOR_BGRA2RGB);
        }
        else {
            cv::cvtColor(mat, rgbFrame, cv::COLOR_BGR2RGB);
        }

        QImage image(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
            rgbFrame.step, QImage::Format_RGB888);

        return QPixmap::fromImage(image.copy());
    }
};
