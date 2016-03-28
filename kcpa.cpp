#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <stdint.h>
#include <modbus/modbus.h>
#include <assert.h>
#include <iostream>
#include "kcpa.h"
#include "ui_kcpa.h"

#define GET_BIT(val,bit) ((val & (1 << bit))?true:false)

template <typename T>
void ShowError(T message)
{
    QMessageBox messageBox;
    messageBox.critical(0,"Error", message);
    messageBox.setFixedSize(500,200);
}

void check_write(int retval){
    if (retval == -1)
        ShowError("VSM not found");
    else if (retval == -2)
        ShowError("Failed to write register or coil. Maybe it does not exist");
    else if (retval == -3)
        ShowError("Unable to connect to VSM port");
}

KCPA::KCPA(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::KCPA)
{
    ui->setupUi(this);
    setupSignals();
}

void KCPA::setupSignals()
{
    connect(ui->psi8_vd1, SIGNAL(clicked()), this, SLOT(ledButtonPressed()));
    connect(ui->psi8_vd2, SIGNAL(clicked()), this, SLOT(ledButtonPressed()));
    connect(ui->psi8_vd3, SIGNAL(clicked()), this, SLOT(ledButtonPressed()));
    connect(ui->psi8_vd4, SIGNAL(clicked()), this, SLOT(ledButtonPressed()));
    connect(ui->psi8_vd5, SIGNAL(clicked()), this, SLOT(ledButtonPressed()));

    connect(ui->s4m21_m1, SIGNAL(clicked()), this, SLOT(motorButtonPressed()));
    connect(ui->s4m21_m2, SIGNAL(clicked()), this, SLOT(motorButtonPressed()));
    connect(ui->s4m22_m1, SIGNAL(clicked()), this, SLOT(motorButtonPressed()));
    connect(ui->s4m22_m2, SIGNAL(clicked()), this, SLOT(motorButtonPressed()));

    connect(ui->pk1i1, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk1i2, SIGNAL(clicked()), this, SLOT(circuitChecked()));

    connect(ui->pk4b_v1o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4b_v2o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4b_v3o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4b_v4o, SIGNAL(clicked()), this, SLOT(circuitChecked()));

    connect(ui->pk4r1_v1o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4r1_v2o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4r1_v3o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4r1_v4o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4r2_v1o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4r2_v2o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4r2_v3o, SIGNAL(clicked()), this, SLOT(circuitChecked()));
    connect(ui->pk4r2_v4o, SIGNAL(clicked()), this, SLOT(circuitChecked()));

    setupUI();

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(updateDeviceStatus()));
    timer->start(1000);
    return;
}

void KCPA::setEnabledPK4R1(bool enabled)
{
    ui->pk4r1_v1o->setEnabled(enabled);
    ui->pk4r1_v2o->setEnabled(enabled);
    ui->pk4r1_v3o->setEnabled(enabled);
    ui->pk4r1_v4o->setEnabled(enabled);
}

void KCPA::setEnabledPK4R2(bool enabled)
{
    ui->pk4r2_v1o->setEnabled(enabled);
    ui->pk4r2_v2o->setEnabled(enabled);
    ui->pk4r2_v3o->setEnabled(enabled);
    ui->pk4r2_v4o->setEnabled(enabled);
}

void KCPA::setEnabledSM1(bool enabled)
{
    ui->s4m21_ccw->setEnabled(enabled);
    ui->s4m21_cw->setEnabled(enabled);
    ui->s4m21_steps->setEnabled(enabled);
    ui->s4m21_m1->setEnabled(enabled);
    ui->s4m21_m2->setEnabled(enabled);
}

void KCPA::setEnabledSM2(bool enabled)
{
    ui->s4m22_ccw->setEnabled(enabled);
    ui->s4m22_cw->setEnabled(enabled);
    ui->s4m22_steps->setEnabled(enabled);
    ui->s4m22_m1->setEnabled(enabled);
    ui->s4m22_m2->setEnabled(enabled);
}


void KCPA::setupUI()
{
    //Determine edition
    //0 = base, 1 = 1SM, 2 = 2SM
    bool sm1;
    bool sm2;
    int retval;
    int edition = 0;

    do {
        retval = writeCoil(0x4, true);
    } while( retval == -3);

    if (retval == -2) sm1 = true;
    else {
        writeCoil(0x4, false);
        sm1 = false;
    }

    do {
        retval = writeCoil(0x8, true);
    } while( retval == -3);

    if (retval == -2) sm2 = true;
    else {
        writeCoil(0x8, false);
        sm2 = false;
    }

    if((sm1 == false) && (sm2 == false)) {
        edition = 0;
    }
    else if((sm1 == true) && (sm2 == true)) {
        edition = 2;
    }
    else if((sm1 == true) || (sm2 == true)) {
        edition = 1;
    }

    if (edition == 0) {
        setEnabledSM1(false);
        setEnabledSM2(false);
        setEnabledPK4R1(true);
        setEnabledPK4R2(true);
    }
    else if (edition == 2) {
        setEnabledSM1(true);
        setEnabledSM2(true);
        setEnabledPK4R1(false);
        setEnabledPK4R2(false);
    }
    else if (edition == 1) {
        setEnabledSM1(true);
        setEnabledSM2(false);
        setEnabledPK4R1(false);
        setEnabledPK4R2(true);
    }
}

void KCPA::updateDeviceStatus()
{
    uint16_t coils;
    int error = readCoils(&coils);
    if (error >= 0) {
        ui->pk4b_v1o->setChecked(GET_BIT(coils, 0x0));
        ui->pk4b_v2o->setChecked(GET_BIT(coils, 0x1));
        ui->pk4b_v3o->setChecked(GET_BIT(coils, 0x2));
        ui->pk4b_v4o->setChecked(GET_BIT(coils, 0x3));

        ui->pk4r1_v1o->setChecked(GET_BIT(coils, 0x4));
        ui->pk4r1_v2o->setChecked(GET_BIT(coils, 0x5));
        ui->pk4r1_v3o->setChecked(GET_BIT(coils, 0x6));
        ui->pk4r1_v4o->setChecked(GET_BIT(coils, 0x7));

        ui->pk4r2_v1o->setChecked(GET_BIT(coils, 0x8));
        ui->pk4r2_v2o->setChecked(GET_BIT(coils, 0x9));
        ui->pk4r2_v3o->setChecked(GET_BIT(coils, 0xA));
        ui->pk4r2_v4o->setChecked(GET_BIT(coils, 0xB));

        ui->pk1i1->setChecked(GET_BIT(coils, 0xC));
        ui->pk1i2->setChecked(GET_BIT(coils, 0xD));
    }
}

void KCPA::circuitChecked()
{
    QString circname = sender()->objectName();
    if (circname == "pk1i1") {
        check_write(writeCoil(0xC, ui->pk1i1->isChecked()));
    }
    else if (circname == "pk1i2") {
        check_write(writeCoil(0xD, ui->pk1i2->isChecked()));
    }
    else if (circname == "pk4b_v1o") {
        check_write(writeCoil(0x0, ui->pk4b_v1o->isChecked()));
    }
    else if (circname == "pk4b_v2o") {
        check_write(writeCoil(0x1, ui->pk4b_v2o->isChecked()));
    }
    else if (circname == "pk4b_v3o") {
        check_write(writeCoil(0x2, ui->pk4b_v3o->isChecked()));
    }
    else if (circname == "pk4b_v4o") {
        check_write(writeCoil(0x3, ui->pk4b_v4o->isChecked()));
    }
    else if (circname == "pk4r1_v1o") {
        check_write(writeCoil(0x4, ui->pk4r1_v1o->isChecked()));
    }
    else if (circname == "pk4r1_v2o") {
        check_write(writeCoil(0x5, ui->pk4r1_v2o->isChecked()));
    }
    else if (circname == "pk4r1_v3o") {
        check_write(writeCoil(0x6, ui->pk4r1_v3o->isChecked()));
    }
    else if (circname == "pk4r1_v4o") {
        check_write(writeCoil(0x7, ui->pk4r1_v4o->isChecked()));
    }
    else if (circname == "pk4r2_v1o") {
        check_write(writeCoil(0x8, ui->pk4r2_v2o->isChecked()));
    }
    else if (circname == "pk4r2_v2o") {
        check_write(writeCoil(0x9, ui->pk4r2_v2o->isChecked()));
    }
    else if (circname == "pk4r2_v3o") {
        check_write(writeCoil(0xA, ui->pk4r2_v3o->isChecked()));
    }
    else if (circname == "pk4r2_v4o") {
        check_write(writeCoil(0xB, ui->pk4r2_v4o->isChecked()));
    }
}

void KCPA::motorButtonPressed()
{
    uint16_t address;
    int section = -1;
    uint16_t value = 0;
    uint16_t steps;
    QString motor = sender()->objectName();
    if (motor == "s4m21_m1") {
        section = 0;
        address = 0x11;
    }
    else if (motor == "s4m21_m2") {
        section = 0;
        address = 0x12;
    }
    else if (motor == "s4m22_m1") {
        section = 1;
        address = 0x21;
    }
    else if (motor == "s4m22_m2") {
        section = 1;
        address = 0x22;
    }
    else {
        /* NOT REACHED */
        assert(false);
    }

    QString step_text;
    if (section == 0) {
        if (ui->s4m21_ccw->isChecked()) value |= (1 << 8);
        step_text = ui->s4m21_steps->text();

    }
    else if (section == 1) {
        if (ui->s4m22_ccw->isChecked()) value |= (1 << 8);
        step_text = ui->s4m22_steps->text();
    }
    else {
        /* NOT REACHED */
        assert(false);
    }

    bool ok;
    int step_count = step_text.toInt(&ok);
    if (!ok) {
        ShowError("Invalid value: not a number!");
        return;
    }
    if ((step_count < 0) || (step_count > 255)) {
        ShowError("Invalid value: nefative or >255  number!");
        return;
    }
    steps = static_cast<uint16_t>(step_count);
    value |= steps;
    int retval = writeRegister(address, value);
    if (-1 == retval) ShowError("VSM not found");
    if (-3 == retval) ShowError("Unable to connect to VSM");
}

void KCPA::ledButtonPressed()
{
    uint16_t led_mode;
    uint16_t led_color;
    uint16_t value;
    uint16_t address;
    QString led_name = sender()->objectName();

    // Determine LED
    if (led_name.compare("psi8_vd1") == 0) {
        address = 0x0000;
    } else if (led_name.compare("psi8_vd2") == 0) {
        address = 0x0001;
    } else if (led_name.compare("psi8_vd3") == 0) {
        address = 0x0002;
    } else if (led_name.compare("psi8_vd4") == 0) {
        address = 0x0203;
    } else if (led_name.compare("psi8_vd5") == 0) {
        address = 0x0004;
    } else {
        /* NOT REACHED */
        assert(false);
    }

    // Determine LED mode
    if (ui->psi8_0hz->isChecked()) {
        led_mode = 0x0000;
    } else if (ui->psi8_1hz->isChecked()) {
        led_mode = 0x0100;
    } else if (ui->psi8_2hz->isChecked()) {
        led_mode = 0x0200;
    } else {
        /* NOT REACHED */
        assert(false);
    }

    // Determin LED color
    if (ui->psi8_off->isChecked()) {
        led_color = 0x0000;
    }
    else if (ui->psi8_green->isChecked()) {
        led_color = 0x0001;
    }
    else if (ui->psi8_red->isChecked()) {
        led_color = 0x0002;
    }
    else if (ui->psi8_yellow->isChecked()) {
        led_color = 0x0003;
    }
    else if (ui->psi8_greenred->isChecked()) {
        led_color = 0x0012;
    }
    else if (ui->psi8_greenyellow->isChecked()) {
        led_color = 0x0013;
    }
    else if (ui->psi8_redyellow->isChecked()) {
        led_color = 0x0023;
    }
    else {
        /* NOT REACHED */
        assert(false);
    }

    value = led_mode | led_color;
    std::cout << "LED value " << value << std::endl;
    check_write(writeRegister(address, value));

}

int KCPA::writeRegister(uint16_t addr, uint16_t value) {
    QString portname = "COM2";
    modbus_t *mb = NULL;
    int retval = 0;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        if (VSM_VENDOR_ID == info.vendorIdentifier() &&
            VSM_PRODUCT_ID == info.productIdentifier()) {
                portname = info.portName();
        }
    }
    if (portname.compare("") == 0) {
        /*VSM not found*/
        return -1;
    }

    mb = modbus_new_rtu(portname.toStdString().c_str(), 19200, 'N', 8, 1);
    if (mb != NULL){
        retval = modbus_connect(mb);
        if (retval != -1){
            modbus_set_slave(mb, 0x30);
            if (-1 == modbus_write_register(mb, addr, value)) {
                retval = -2;
            }
            modbus_close(mb);
        }
        else {
            retval = -3;
        }
        modbus_free(mb);
    }
    return retval;
}

int KCPA::writeCoil(uint16_t addr, bool value) {
    QString portname = "COM2";
    modbus_t *mb = NULL;
    int retval = 0;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        if (VSM_VENDOR_ID == info.vendorIdentifier() &&
            VSM_PRODUCT_ID == info.productIdentifier()) {
                portname = info.portName();
        }
    }
    if (portname.compare("") == 0) {
        /*VSM not found*/
        return -1;
    }

    mb = modbus_new_rtu(portname.toStdString().c_str(), 19200, 'N', 8, 1);
    if (mb != NULL){
        retval = modbus_connect(mb);
        if (retval != -1){
            modbus_set_slave(mb, 0x30);
            int val;
            if (value) val = 1;
            else val = 0;
            if (-1 == modbus_write_bit(mb, addr, val)) {
                retval = -2;
            }
            modbus_close(mb);
        }
        else {
            retval = -3;
        }
        modbus_free(mb);
    }
    return retval;
}

int KCPA::readCoils(uint16_t *values)
{
    QString portname = "COM2";
    modbus_t *mb = NULL;
    uint8_t vals[0xE];
    int retval = 0;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        if (VSM_VENDOR_ID == info.vendorIdentifier() &&
            VSM_PRODUCT_ID == info.productIdentifier()) {
                portname = info.portName();
        }
    }
    if (portname.compare("") == 0) {
        /*VSM not found*/
        return -1;
    }

    mb = modbus_new_rtu(portname.toStdString().c_str(), 19200, 'N', 8, 1);
    if (mb != NULL){
        retval = modbus_connect(mb);
        if (retval != -1){
            modbus_set_slave(mb, 0x30);
            if (-1 == modbus_read_bits(mb, 0, 0xE, vals)) {
                retval = -2;
            }
            *values = 0;
            for(int i = 0; i < 0xE; i++) {
                if(vals[i]) {
                    *values |= (1 << i);
                }
            }
            modbus_close(mb);
        }
        else {
            retval = -3;
        }
        modbus_free(mb);
    }
    return retval;
}

KCPA::~KCPA()
{
    delete ui;
}
