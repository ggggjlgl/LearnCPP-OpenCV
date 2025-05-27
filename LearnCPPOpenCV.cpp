#include "LearnCPPOpenCV.h"

LearnCPPOpenCV::LearnCPPOpenCV(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LearnCPPOpenCVClass())
{
    ui->setupUi(this);
}

LearnCPPOpenCV::~LearnCPPOpenCV()
{
    delete ui;
}

