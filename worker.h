#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <atomic>
#include <string>
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
    // ����ԭʼ֡�ͼ����֡
    void updateRes(QPixmap originPixmap, QPixmap resPixmap);
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
                drawDetections(frame, output);
                emit updateRes(originPixmap, cvMatToQPixmap(frame));
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
    void drawDetections(cv::Mat& frame, const std::vector<Detection>& output) {
        for (const auto& detection : output) {
            cv::Rect box = detection.box;
            cv::Scalar color = detection.color;

            cv::rectangle(frame, box, color, 2);

            std::string classString = detection.className + ' ' + std::to_string(detection.confidence).substr(0, 4);
            cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
            cv::Rect textBox(box.x, box.y - 40, textSize.width + 10, textSize.height + 20);

            cv::rectangle(frame, textBox, color, cv::FILLED);
            cv::putText(frame, classString, cv::Point(box.x + 5, box.y - 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
        }
    }
    QPixmap cvMatToQPixmap(const cv::Mat& mat) {
        // ȷ��ͼ����RGB��ʽ
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

        // ����QImage��ת��ΪQPixmap
        QImage image(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
            rgbFrame.step, QImage::Format_RGB888);

        // ��Ҫ�������ݣ���ΪQImage������ԭʼ�ڴ�
        return QPixmap::fromImage(image.copy());
    }
};
