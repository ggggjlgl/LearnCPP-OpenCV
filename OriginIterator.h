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
    virtual int progress() = 0;
    virtual QString origin() = 0;
};

class ImagesIterator : public OriginIteratorBase {
public:

    explicit ImagesIterator(const QStringList& images):m_images(images), m_index(0), m_curImage("") {}

    cv::Mat next() override {
        if (m_index < m_images.size()) {
            m_curImage = m_images[m_index++];
            cv::Mat frame = cv::imread(m_curImage.toLocal8Bit().constData());
            return frame;
        }
        else {
            return cv::Mat();
        }
    }
    int progress() {
        return int(m_index * 100 / m_images.size());
    }
    QString origin() {
        return m_curImage;
    }

private:
    QStringList m_images;
    size_t m_index;
    QString m_curImage;
};

class VideoIterator : public OriginIteratorBase {
public:
    explicit VideoIterator(const QString& videoPath) 
        : m_videoPath(videoPath), m_video(new cv::VideoCapture)
        , m_framePos(0), m_frameStep(0)
    {
        m_video->open(m_videoPath.toStdString());
        if (m_video->isOpened()) {
            m_frameStep = m_video->get(cv::CAP_PROP_FPS) / 4;
        }
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
        m_video->set(cv::CAP_PROP_POS_FRAMES, m_framePos);
        cv::Mat frame;
        if (!m_video->read(frame) || frame.empty()) {
            return cv::Mat();
        }
        m_framePos += m_frameStep;
        return frame;
    }
    int progress() {
        if (!m_video || !m_video->isOpened()) {
            return 100;
        }
        return int(m_video->get(cv::CAP_PROP_POS_FRAMES) * 100 / m_video->get(cv::CAP_PROP_FRAME_COUNT));
    }
    QString origin() {
        return m_videoPath;
    }
    
private:
    QString m_videoPath;
    cv::VideoCapture* m_video;
    int m_framePos;
    double m_frameStep;
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
    int progress() {
        return 100;
    }
    QString origin() {
        return QString::fromStdString(std::to_string(m_cameraIndex));
    }
private:
    int m_cameraIndex;
    cv::VideoCapture* m_camera;
};


#endif // ORIGINITOR_H
