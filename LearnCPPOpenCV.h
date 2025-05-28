#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LearnCPPOpenCV.h"
#include "worker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LearnCPPOpenCVClass; };
QT_END_NAMESPACE

class LearnCPPOpenCV : public QMainWindow
{
    Q_OBJECT

public:
    LearnCPPOpenCV(QWidget *parent = nullptr);
    ~LearnCPPOpenCV();
    void enableButtons(bool state);

private slots:
    void updateRes(QPixmap originPixmap, QPixmap resPixmap);
    void finishWork(std::string err);
    void imagesDetect();
    void videoDetect();
    void cameraDetect();
    void stopDetect();
private:
    Ui::LearnCPPOpenCVClass *ui;
    Worker* m_worker;
    QThread* m_thread;
};

