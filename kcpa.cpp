#include "kcpa.h"
#include "ui_kcpa.h"

KCPA::KCPA(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KCPA)
{
    ui->setupUi(this);
}

KCPA::~KCPA()
{
    delete ui;
}
