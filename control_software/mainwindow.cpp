#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "iostream"
#include "cmath"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QList>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QSettings>

QSerialPort *serial;
double min, max;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(QSize(400, 420));
    QSettings settings("ULL", "Attenuator");
    min = settings.value("min", 0).toDouble();
    max = settings.value("max", 100).toDouble();
    if (min < 1000000.0)
    {
        if (min < 1000)
        {
            ui->lineEdit_min->setText(QString::number(min, 'f', 1));
            ui->combo_unit_0->setCurrentIndex(0);
        }
        else
        {
            ui->lineEdit_min->setText(QString::number(min / 1000, 'f', 1));
            ui->combo_unit_0->setCurrentIndex(1);
        }
    }
    else
    {
        ui->lineEdit_min->setText(QString::number(min / 1000000, 'f', 1));
        ui->combo_unit_0->setCurrentIndex(2);
    }
    if (max < 1000000.0)
    {
        if (max < 1000)
        {
            ui->lineEdit_max->setText(QString::number(max, 'f', 1));
            ui->combo_unit_1->setCurrentIndex(0);
        }
        else
        {
            ui->lineEdit_max->setText(QString::number(max / 1000, 'f', 1));
            ui->combo_unit_1->setCurrentIndex(1);
        }
    }
    else
    {
        ui->lineEdit_max->setText(QString::number(max / 1000000, 'f', 1));
        ui->combo_unit_1->setCurrentIndex(2);
    }

    connect(ui->btn_connect, SIGNAL(released()), this, SLOT(on_btnConnectClicked()));
    connect(ui->btn_refresh,  SIGNAL(released()), this, SLOT(on_btnRefreshClicked()));
    connect(ui->btn_cal,  SIGNAL(released()), this, SLOT(on_btnCalibrateClicked()));
    connect(ui->btn_move,  SIGNAL(released()), this, SLOT(on_btnMoveClicked()));
    connect(ui->powerSlider, SIGNAL(valueChanged(int)), this, SLOT(on_sliderMoved(int)));
    ui->frame_control->setEnabled(false);
    ui->frame_calibration->setEnabled(false);
    ui->statusbar->showMessage("Disconnected");
    MainWindow::on_btnRefreshClicked();
    serial = new QSerialPort(this);
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(serial, SIGNAL(readyRead()), this, SLOT(on_serialReceived()));
}

MainWindow::~MainWindow()
{
    delete ui;
    if(serial->isOpen()){
        serial->close();
    }
}

void MainWindow::on_btnConnectClicked()
{
    QString selected = ui->combo_ports->currentText();

    if (selected != "")
    {
        //Close the port if it's already open
        if(serial->isOpen()){
            serial->close();
            ui->statusbar->showMessage("Disconnected");
        }
        //set the port name
        serial->setPortName(selected);
        if (serial->open(QIODevice::ReadWrite))
        {
            serial->setDataTerminalReady(true);
            serial->setRequestToSend(true);
            std::cout<<"Port Opened"<<std::endl;
        }else {
            QMessageBox::critical(this, "Error",
                          "Cannot open the port\n"+serial->errorString(),
                                                        QMessageBox::Ok);
            qCritical() << "Serial Port error:" << serial->errorString();
        }
        QThread::msleep(1500);
        serial->write("connect");
        serial->flush();
    }
}

void MainWindow::on_serialReceived()
{

    QString response = serial->readLine();
    response.remove(QRegExp("[\n\r]"));
    qDebug() << "receiving..\n"+response;

    if (response == "ok")
    {
        if (ui->statusbar->currentMessage() == "Disconnected")
            ui->statusbar->showMessage("Homing...");
        else
            ui->statusbar->showMessage("Ready");
    }
    else
    {
        ui->statusbar->showMessage("Ready");
        ui->frame_control->setEnabled(true);
        ui->frame_calibration->setEnabled(true);
        ui->powerSlider->setValue(response.toInt());
        MainWindow::on_sliderMoved(response.toInt());
    }
 }

void MainWindow::on_btnRefreshClicked()
{
    QList<QSerialPortInfo> list;
    list = QSerialPortInfo::availablePorts();

    //clear the combo box
    ui->combo_ports->clear();
    foreach(const QSerialPortInfo &serial, list)
    {
        //add the available port
        ui->combo_ports->addItem(serial.portName());
    }
}

void MainWindow::on_btnCalibrateClicked()
{
    int min_factor = std::pow(1000, ui->combo_unit_0->currentIndex());
    int max_factor = std::pow(1000, ui->combo_unit_1->currentIndex());
    double temp_min = ui->lineEdit_min->text().toDouble() * min_factor;
    double temp_max = ui->lineEdit_max->text().toDouble() * max_factor;

    if (temp_max == 0 || temp_max <= temp_min)
        QMessageBox::critical(this, "Error", "Wrong values");
    else if (temp_min < 0 || temp_max < 0)
        QMessageBox::critical(this, "Error",
                              "Only positive values are allowed");
    else
    {
        min = temp_min;
        max = temp_max;
        ui->statusbar->showMessage("Recalibrated");
        MainWindow::on_sliderMoved(ui->powerSlider->value());
    }
}

void MainWindow::on_sliderMoved(int value)
{
    QString pct, energy;
    double current_energy;
    current_energy = min + (max - min) * value / 100;
    QTextStream(&pct) << value << "%";
    if (current_energy < 1000000.0)
    {
        if (current_energy < 1000)
            energy = QString::number(current_energy, 'f', 1) + "uJ";
        else
            energy = QString::number(current_energy / 1000, 'f', 1) + "mJ";
    }
    else
        energy = QString::number(current_energy / 1000000, 'f', 1) + "J";

    ui->lbl_pct->setText(pct);
    ui->lbn->setText(energy);

}

void MainWindow::on_btnMoveClicked()
{
    if (serial->isOpen())
    {
        serial->write(std::to_string(ui->powerSlider->value()).c_str());
        ui->statusbar->showMessage("Moving...");
    }
    else
    {
        ui->frame_control->setEnabled(false);
        ui->frame_calibration->setEnabled(false);
        ui->statusbar->showMessage("Disconnected");
    }
}

void MainWindow::closeEvent(QCloseEvent * event)
{
   QSettings settings("ULL", "Attenuator");
   settings.setValue("min", min);
   settings.setValue("max", max);
   settings.sync();

}
