#ifndef KCPA_H
#define KCPA_H

#include <QMainWindow>

namespace Ui {
class KCPA;
}

class KCPA : public QMainWindow
{
    Q_OBJECT

public:
    explicit KCPA(QWidget *parent = 0);
    ~KCPA();

private:
    Ui::KCPA *ui;
};

#endif // KCPA_H
