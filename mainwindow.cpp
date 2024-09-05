#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "actuator_lib.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <QDebug>
#include <fstream>
#include <QFileDialog>
#include <QUrl>
#include <QMessageBox>
#include <QStorageInfo>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QThread>

const double ZeroOneThresh = 0.42;
const double OneTwoThresh = 1.01;
const double TwoThreeThresh = 1.44;
const double ThreeFourThresh = 1.82;
const double FourFiveThresh = 2.12;
const double FiveSixThresh = 2.38;
const double SixSevenThresh = 2.59;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    deviceID = 1;
    axisInd = 1;

    connect(&mytimer, SIGNAL(timeout()), this, SLOT(mytimerhandler()));
}

MainWindow::~MainWindow()
{
    delete ui;
    // delete actuator;
}

/**
 * @brief MainWindow::mytimerhandler
 * This function is called on a timer and updates the following on the GUI: 
 * 1. Current Position
 * 2. Actuator Status (Busy/Idle)
 * 3. Actuator Homed Status
 * 4. Imaging Probe Status
 * 5. Pico Temperature
*/
void MainWindow::mytimerhandler()
{
    std::vector<double> curPos;
    curPos = actuator->getPosition(axisInd);

    ui->label_CurrentPosition->setText(QString::number(curPos.back()));

    if (actuator->isBusy(axisInd))
        ui->label_6->setText("Busy");

    else
    {

        ui->label_6->setText("Idle");
    }

    if (actuator->isHomed(axisInd))
        ui->label_8->setText("Homed");

    else
    {

        ui->label_8->setText("Not Homed");
    }

    ui->label_GPIO27Analog->setText(actuator->getImagingProbe().c_str());

    ui->label_13->setText(QString::number(actuator->getPicoTemperature()));
}

void MainWindow::on_pushButton_Home_clicked()
{
    // std::cout << "Homing Actuator without wait" << std::endl;
    qDebug() << "Homed:" << actuator->isHomed(axisInd);
    qDebug() << "Homing Actuator without wait\n";
    double vel;
    vel = ui->lineEdit_Velocity->text().toDouble();

    if (!actuator->home(axisInd, vel, false))
    {
        qDebug() << "ERROR: Could not home actuator\n";
    }
}

void MainWindow::on_pushButton_MoveAbs_clicked()
{
    double pos, vel, accel;

    pos = ui->lineEdit_AbsPosition->text().toDouble();
    vel = ui->lineEdit_Velocity->text().toDouble();
    accel = ui->lineEdit_Acceleration->text().toDouble();

    bool waitUntilIdle = ui->moveAbsBox->isChecked();
    if(waitUntilIdle)
    {
        qDebug() << "Moving Actuator to " << pos << "mm with wait\n";
    }
    else
    {
        qDebug() << "Moving Actuator to " << pos << "mm without wait\n";
    }

    if (!actuator->moveAbsolute(pos, axisInd, vel, accel, ui->moveAbsBox->isChecked()))
    {
        qDebug() << "ERROR: Could not move actuator\n";
    }
    qDebug() << "Actuator Moved\n";
}

void MainWindow::on_pushButton_MoveRel_clicked()
{
    double dis, vel, accel;

    dis = ui->lineEdit_RelDistance->text().toDouble();
    vel = ui->lineEdit_Velocity->text().toDouble();
    accel = ui->lineEdit_Acceleration->text().toDouble();

    bool waitUntilIdle = ui->moveRelBox->isChecked();
    if(waitUntilIdle)
    {
        qDebug() << "Moving Actuator by " << dis << "mm with wait\n";
    }
    else
    {
        qDebug() << "Moving Actuator by " << dis << "mm without wait\n";
    }

    if (!actuator->moveRelative(dis, axisInd, vel, accel, ui->moveRelBox->isChecked()))
    {
        qDebug() << "ERROR: Could not move actuator\n";
    }
    qDebug() << "Actuator Moved\n";
}

void MainWindow::on_pushButton_Connect_clicked()
{
    std::string comPort = "";
    actuator = new Actuator(deviceID, comPort);

    if (!actuator->connect())
    {
        qDebug() << "ERROR: Could not connect to actuator\n";
    }
    else
    {
        qDebug() << "Actuator Connected\n";
        mytimer.start(20);  // Start the timer to update the GUI
    }

    actuator->setDirection(0, axisInd);

    // get the firmware version and put it on GUI
    std::string FirmwareVersion = actuator->getFirmwareVersion();
    ui->label_12->setText("Firmware Version " + QString::fromStdString(FirmwareVersion));
}

void MainWindow::on_pushButton_Disconnect_clicked()
{
    std::vector<double> lastPos = actuator->getPosition(axisInd);
    char filename[] = "savedPos.txt";
    std::ifstream infile(filename);
    std::string myText = std::to_string(lastPos.back());
    if (infile.good())
    {
        std::ofstream MyFile;
        MyFile.open(filename);
        MyFile << myText;
        MyFile.close();
    }
    mytimer.stop();
    
    qDebug() << "Disconnecting Actuator\n";
    if (!actuator->disconnect())
    {
        qDebug() << "ERROR: Could not disconnect from actuator\n";
        return;
    }
    delete actuator;
    actuator=nullptr;
    qDebug() << "Actuator disconnected\n";
}

void MainWindow::on_pushButton_Stop_clicked()
{
    actuator->stop(axisInd, 0);
}

void MainWindow::on_onBtn_clicked()
{
    actuator->setGPIO(14, 1);
}

void MainWindow::on_offBtn_clicked()
{
    actuator->setGPIO(14, 0);
}

void MainWindow::on_setPosBtn_clicked()
{
    qDebug() << "Actuator is homed:" << actuator->isHomed(axisInd);
    qDebug() << "Setting Position\n";
    double pos;
    pos = ui->lineEdit_setPos->text().toDouble();
    if (!actuator->setPosition(pos, axisInd))
    {
        qDebug() << "ERROR: Position could not be set\n";
    }
    qDebug() << "Actuator Position Set\n";

    qDebug() << "Actuator is homed:" << actuator->isHomed(axisInd);
}

void MainWindow::on_fileinput_clicked()
{
    QString sourcefilename =QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "",
        "All files(*.*);;Text File (*.txt);;"
        );

    if (!sourcefilename.isEmpty()) {

        // Get a list of all available volumes
        QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();

        // Iterate through each volume to find the removable disks
        foreach(const QStorageInfo &storage, volumes) {
            if (storage.isValid()) {
                if (storage.displayName() == "RPI-RP2")
                {
                QString destinationDirectory = storage.rootPath();
                QString fileNameOnly = QFileInfo(sourcefilename).fileName();
                QString destinationFilePath = QDir(destinationDirectory).filePath(fileNameOnly);
                // Copy the file
                if (QFile::copy(sourcefilename, destinationFilePath)) {
                    qDebug() << "File copied successfully to" << destinationDirectory;
                    QThread::sleep(1) ;
                    // MainWindow::on_pushButton_Connect_clicked();
                    return;
                } else {
                    qDebug() << "Error copying file to" << destinationDirectory;
                }
                }
            }
        }
    }
    else
    {
        qDebug() << "No removable disks found.";
    }

}

void MainWindow::on_reset_clicked()
{
    if (actuator)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Upload Firmware", "First select the firmware file to upload",
                                      QMessageBox::Ok | QMessageBox::Cancel );
        if (reply == QMessageBox::Ok)
        {
            actuator->resetPi();
            MainWindow::on_pushButton_Disconnect_clicked();
            QThread::msleep(200) ;
            on_fileinput_clicked();
            QThread::msleep(1500) ;
            MainWindow::on_pushButton_Connect_clicked();
        }

    }
}
