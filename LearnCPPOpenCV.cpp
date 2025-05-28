#include <string>
#include <vector>
#include <chrono>
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
    setupWidgets();
    m_worker->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_worker, &Worker::doWork);
    connect(m_worker, &Worker::updateRes, this, &LearnCPPOpenCV::updateRes);
    connect(m_worker, &Worker::updateProgress, this, &LearnCPPOpenCV::updateProgress);
    connect(m_worker, &Worker::updateRecords, this, &LearnCPPOpenCV::updateRecords);
    connect(m_worker, &Worker::finished, this, &LearnCPPOpenCV::finishWork);

    connect(ui->pbImages, &QPushButton::clicked, this, &LearnCPPOpenCV::imagesDetect);
    connect(ui->pbVideo, &QPushButton::clicked, this, &LearnCPPOpenCV::videoDetect);
    connect(ui->pbCamera, &QPushButton::clicked, this, &LearnCPPOpenCV::cameraDetect);
    connect(ui->pbStop, &QPushButton::clicked, this, &LearnCPPOpenCV::stopDetect);
}

LearnCPPOpenCV::~LearnCPPOpenCV()
{
    stopThread();
    delete ui;
}
void LearnCPPOpenCV::setupWidgets() {
    ui->table->setColumnWidth(0, 150);
    ui->table->setColumnWidth(1, 100);
    ui->table->setColumnWidth(2, 300);
    ui->table->setColumnWidth(3, 100);
    ui->table->setColumnWidth(4, 160);
}
void LearnCPPOpenCV::stopThread() {
    if (m_worker) {
        m_worker->stopWork();
    }
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait();
    }
}
void LearnCPPOpenCV::updateRecords(QString origin, std::vector<std::map<QString, QString>> detected) {
    ui->lbTarget->setText("目标类型：" + detected[0]["class"]);
    ui->lbArea->setText("区域：" + detected[0]["area"]);
    ui->lbConf->setText("置信度：" + detected[0]["confidence"]);
    for (const auto& target : detected) {
        int row = ui->table->rowCount();
        ui->table->insertRow(row);
        ui->table->setItem(row, 0, new QTableWidgetItem(origin));
        ui->table->setItem(row, 1, new QTableWidgetItem(target.at("class")));
        ui->table->setItem(row, 2, new QTableWidgetItem(target.at("area")));
        ui->table->setItem(row, 3, new QTableWidgetItem(target.at("confidence")));
        ui->table->setItem(row, 4, new QTableWidgetItem(getTimeStr()));
        ui->table->setItem(row, 5, new QTableWidgetItem("/"));
    }
    ui->table->scrollToBottom();
    
}
void LearnCPPOpenCV::updateProgress(int progress) {
    ui->pgb->setValue(progress);
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

QString LearnCPPOpenCV::getTimeStr() {
    QDateTime currentDateTime = QDateTime::currentDateTime();
    return currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
}

void LearnCPPOpenCV::finishWork(std::string err) {
    if (!err.empty()) {
        QMessageBox::information(this, "Tip", QString::fromStdString(err));
    }
    enableButtons(true);
}

void LearnCPPOpenCV::imagesDetect() {
    QString folderPath = QFileDialog::getExistingDirectory(this, "select a folder", "");
    if (!folderPath.isEmpty()) {
        QDir dir(folderPath);
        QStringList filters = { "*.jpg", "*.png" };
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
        if (files.empty()) {
            QMessageBox::information(this, "Tip", "No image files under selected folder");
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
    QString videoPath = QFileDialog::getOpenFileName(this, "select a video file", "", "Video File(*.mp4 *.avi)");
    if (!videoPath.isEmpty()) {
        auto iterator = QSharedPointer<OriginIteratorBase>(new VideoIterator(videoPath));
        m_worker->setData(iterator);
        enableButtons(false);
        m_thread->start();
    }
}
void LearnCPPOpenCV::cameraDetect() {
    auto iterator = QSharedPointer<OriginIteratorBase>(new CameraIterator(0));
    m_worker->setData(iterator);
    enableButtons(false);
    m_thread->start();
}
void LearnCPPOpenCV::stopDetect() {
    stopThread();
    enableButtons(true);
    ui->pgb->setValue(0);
}