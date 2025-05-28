#include <string>
#include <vector>
#include <QFileDialog>
#include <QFileInfoList>
#include <QMessageBox>
#include <qDebug>
#include "LearnCPPOpenCV.h"
#include "OriginIterator.h"

LearnCPPOpenCV::LearnCPPOpenCV(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LearnCPPOpenCVClass())
    , m_worker(new Worker(this))
    , m_thread(new QThread(this))
{
    ui->setupUi(this);
    m_worker->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_worker, &Worker::doWork);
    connect(m_worker, &Worker::updateRes, this, &LearnCPPOpenCV::updateRes);
    connect(m_worker, &Worker::finished, this, &LearnCPPOpenCV::finishWork);

    connect(ui->pbImages, &QPushButton::clicked, this, &LearnCPPOpenCV::imagesDetect);
    connect(ui->pbVideo, &QPushButton::clicked, this, &LearnCPPOpenCV::videoDetect);
    connect(ui->pbCamera, &QPushButton::clicked, this, &LearnCPPOpenCV::cameraDetect);
    connect(ui->pbStop, &QPushButton::clicked, this, &LearnCPPOpenCV::stopDetect);
}

LearnCPPOpenCV::~LearnCPPOpenCV()
{
    if (m_worker) {
        m_worker->stopWork();
    }
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait();
    }
    delete ui;
}

void LearnCPPOpenCV::updateRes(QPixmap originPixmap, QPixmap resPixmap) {
    originPixmap = originPixmap.scaled(ui->lbOrigin->size(), Qt::KeepAspectRatio);
    resPixmap = resPixmap.scaled(ui->lbRes->size(), Qt::KeepAspectRatio);
    ui->lbOrigin->setPixmap(originPixmap);
    ui->lbRes->setPixmap(resPixmap);
}


void LearnCPPOpenCV::enableButtons(bool state) {
    ui->pbImages->setEnabled(state);
    ui->pbVideo->setEnabled(state);
    ui->pbCamera->setEnabled(state);
    ui->pbStop->setEnabled(!state);
}

void LearnCPPOpenCV::finishWork(std::string err) {
    if (!err.empty()) {
        QMessageBox::information(this, "提示", QString::fromStdString(err));
    }
    enableButtons(true);
}

void LearnCPPOpenCV::imagesDetect() {
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择图片目录", "");
    if (!folderPath.isEmpty()) {
        QDir dir(folderPath);
        QStringList filters = { "*.jpg", "*.png" };
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
        if (files.empty()) {
            QMessageBox::information(this, "提示", "当前目录下没有有效的图片文件");
            return;
        }
        QStringList images;
        foreach(const QFileInfo & file, files) {
            images.push_back(file.absoluteFilePath());
        }
        auto iterator = QSharedPointer<OriginIteratorBase>(new ImagesIterator(images));
        m_worker->setData(iterator);
        enableButtons(false);
        m_thread->start();
    }
}

void LearnCPPOpenCV::videoDetect() {

}
void LearnCPPOpenCV::cameraDetect() {

}
void LearnCPPOpenCV::stopDetect() {

}