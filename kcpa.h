#ifndef KCPA_H
#define KCPA_H

#include <QMainWindow>
#include <QTimer>
#include <stdint.h>

#define VSM_VENDOR_ID 0x0483
#define VSM_PRODUCT_ID 0x5740

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
    QTimer *timer;
    void setupSignals();
    void setupUI();
    void setEnabledPK4R1(bool);
    void setEnabledPK4R2(bool);
    void setEnabledSM1(bool);
    void setEnabledSM2(bool);
    int writeRegister(uint16_t addr, uint16_t value);
    int writeCoil(uint16_t addr, bool value);
    int readCoils(uint16_t *values);
    Ui::KCPA *ui;

public slots:
    void ledButtonPressed();
    void motorButtonPressed();
    void circuitChecked();
    void updateDeviceStatus();
};

#endif // KCPA_H
