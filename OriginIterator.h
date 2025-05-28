#ifndef ORIGINITOR_H
#define ORIGINITOR_H


#include <string>
#include <vector>
#include <QString>
#include <QStringList>
#include <opencv2/opencv.hpp>


class OriginIteratorBase {
public:
    virtual ~OriginIteratorBase() {}
    virtual cv::Mat next() = 0;
};

class ImagesIterator : public OriginIteratorBase {
public:

    explicit ImagesIterator(const QStringList& images):m_images(images), m_index(0) {}

    cv::Mat next() override {
        if (m_index < m_images.size()) {
            cv::Mat frame = cv::imread(m_images[m_index++].toLocal8Bit().constData());
            return frame;
        }
        else {
            return cv::Mat();
        }
    }

private:
    QStringList m_images;
    size_t m_index;
};

class VideoIterator : public OriginIteratorBase {
public:
    explicit VideoIterator(const QString& videoPath) : m_videoPath(videoPath), m_video(new cv::VideoCapture) {
        m_video->open(m_videoPath.toStdString());
    }
    ~VideoIterator() override {
        if (m_video && m_video->isOpened()) {
            m_video->release();
        }
        delete m_video;
    }
    cv::Mat next() override {
        if (!m_video || !m_video->isOpened()) {
            return cv::Mat();
        }
        cv::Mat frame;
        if (!m_video->read(frame) || frame.empty()) {
            return cv::Mat();
        }
        return frame;
    }
    
private:
    QString m_videoPath;
    cv::VideoCapture* m_video;
};

class CameraIterator : public OriginIteratorBase {
public:
    explicit CameraIterator(int camera_index = 0) : m_cameraIndex(camera_index), m_camera(new cv::VideoCapture) {
        m_camera->open(camera_index);
    }
    ~CameraIterator() override {
        if (m_camera && m_camera->isOpened()) {
            m_camera->release();
        }
        delete m_camera;
    }
    cv::Mat next() override {
        if (!m_camera || !m_camera->isOpened()) {
            return cv::Mat();
        }
        cv::Mat frame;
        if (!m_camera->read(frame) || frame.empty()) {
            return cv::Mat();
        }
        return frame;
    }

private:
    int m_cameraIndex;
    cv::VideoCapture* m_camera;
};


#endif // ORIGINITOR_H
