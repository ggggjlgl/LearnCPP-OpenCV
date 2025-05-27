#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LearnCPPOpenCV.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LearnCPPOpenCVClass; };
QT_END_NAMESPACE

class LearnCPPOpenCV : public QMainWindow
{
    Q_OBJECT

public:
    LearnCPPOpenCV(QWidget *parent = nullptr);
    ~LearnCPPOpenCV();

private:
    Ui::LearnCPPOpenCVClass *ui;
};

