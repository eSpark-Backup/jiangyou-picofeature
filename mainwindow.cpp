#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "actuator_lib.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QUrl>
#include <QMessageBox>
#include <QStorageInfo>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

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

    //
}

MainWindow::~MainWindow()
{
    delete ui;
    // delete actuator;
}

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

    //   qDebug()<<actuator->getGPIO(14);

    double vol;
    vol = double(actuator->getAnalog(1)) / 4096 * 3.3;

    // Adding threshold conditionals

    /*if (vol < ZeroOneThresh) {
        qDebug() << "Zero";
    } else if (vol >= ZeroOneThresh && vol < OneTwoThresh) {
        qDebug() << "One";
    } else if (vol >= OneTwoThresh && vol < TwoThreeThresh) {
        qDebug() << "Two";
    } else if (vol >= TwoThreeThresh && vol < ThreeFourThresh) {
        qDebug() << "Three";
    } else if (vol >= ThreeFourThresh && vol < FourFiveThresh) {
        qDebug() << "Four";
    } else if (vol >= FourFiveThresh && vol < FiveSixThresh) {
        qDebug() << "Five";
    } else if (vol >= FiveSixThresh && vol < SixSevenThresh) {
        qDebug() << "Six";
    } else {
        qDebug() << "Seven";
    }*/

    //   qDebug()<<vol;
    ui->label_GPIO27Analog->setText(QString::number(vol));
    //   if(count%5==0){

    //       vol_avg=vol_avg/5;
    //       qDebug()<<vol_avg;
    //       ui->label_GPIO27Analog->setText(QString::number(vol_avg));
    //       vol_avg=0;

    //   }

    //   else {

    //        vol_avg=vol_avg+vol;
    //   }

    count++;

// Create a thread for updating temperature sensor data
double temperature = actuator->getPicoTemperature();
ui->label_13->setText(QString::number(temperature));
}

void MainWindow::on_pushButton_Home_clicked()
{
    // std::cout << "Homing Actuator without wait" << std::endl;
    qDebug() << "Homing Actuator without wait\n";
    qDebug() << "Actuator is homed:" << actuator->isHomed(axisInd);
    double vel;
    vel = ui->lineEdit_Velocity->text().toDouble();

    if (!actuator->home(axisInd, vel, false))
    {
        // std::cout << "ERROR: Could not home actuator" << std::endl;
    }
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    //  std::cout << "Actuator Homed" << std::endl;

    // std::cout << "Confirming Actuator is homed" << std::endl;

    //    if(!(actuator->isHomed(axisInd))){
    //        // std::cout << "ERROR: Actuator is not homed" << std::endl;
    //             qDebug()<<"ERROR: Actuator is not homed dfgdgdfgdg\n";
    //    }
    // std::cout << "Actuator is homed" << std::endl;.

    qDebug() << "Actuator is homed:" << actuator->isHomed(axisInd);
    qDebug() << "Actuator is homed dddd:" << actuator->isHomed(axisInd);
}

void MainWindow::on_pushButton_MoveAbs_clicked()
{
    // std::cout << "Moving Actuator 10mm without wait" << std::endl;
    qDebug() << "Moving Actuator 10mm without wait\n";
    // moveAbsolute(double position, int axis = 0, double speed = 0, double acceleration = 0, bool waitUntilIdle = false);
    double pos, vel, accel;

    pos = ui->lineEdit_AbsPosition->text().toDouble();
    vel = ui->lineEdit_Velocity->text().toDouble();
    accel = ui->lineEdit_Acceleration->text().toDouble();
    if (!actuator->moveAbsolute(pos, axisInd, vel, accel, ui->moveAbsBox->isChecked()))
    {
        // std::cout << "ERROR: Could not move actuator" << std::endl;
        qDebug() << "ERROR: Could not move actuator\n";
    }
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    //  std::cout << "Actuator Moved" << std::endl;
    qDebug() << "Actuator Moved\n";
}

void MainWindow::on_pushButton_MoveRel_clicked()
{
    // std::cout << "Moving Actuator 10mm without wait" << std::endl;
    qDebug() << "Moving Actuator 10mm without wait\n";
    // moveAbsolute(double position, int axis = 0, double speed = 0, double acceleration = 0, bool waitUntilIdle = false);
    double dis, vel, accel;

    dis = ui->lineEdit_RelDistance->text().toDouble();
    vel = ui->lineEdit_Velocity->text().toDouble();
    qDebug() << "speed is " << vel << "mm/s";
    accel = ui->lineEdit_Acceleration->text().toDouble();
    if (!actuator->moveRelative(dis, axisInd, vel, accel, ui->moveRelBox->isChecked()))
    {
        // std::cout << "ERROR: Could not move actuator" << std::endl;
        qDebug() << "ERROR: Could not move actuator\n";
    }
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    // std::cout << "Actuator Moved" << std::endl;
    qDebug() << "Actuator Moved\n";
}
void MainWindow::on_pushButton_Connect_clicked()
{
    // qDebug()<<"Connection Status: "<<actuator->isConnected();
    std::string comPort = "COM4";
    actuator = new Actuator(deviceID, comPort);

    // qDebug()<<"Connection Status: "<<actuator->isConnected();

    //   std::mutex * stdioMutex = actuator->enableDebug(true);

    if (!actuator->connect())
    {
        // std::cout << "ERROR: Could not connect to actuator" << std::endl;
        // consoleOut(stdioMutex, "ERROR: Could not connect to actuator\n");
        // return 1;
        qDebug() << "ERROR: Could not connect to actuator\n";
    }
    else
    {

        // std::cout << "Actuator Connected" << std::endl;
        qDebug() << "Actuator Connected\n";
        mytimer.start(20);
    }
    actuator->setDirection(0, axisInd);
    // get the firmware version and put it on GUI
    std::string FirmwareVersion = actuator->getFirmwareVersion();
    ui->label_12->setText("Firmware Version " + QString::fromStdString(FirmwareVersion));

    // Create a thread for updating temperature sensor data
    double temperature = actuator->getPicoTemperature();
    ui->label_13->setText(QString::number(temperature));

    //    int axisCount = actuator->getAxisCount();
    //    //qDebug()<<"Turn Pin 10 On";
    //    qDebug()<<"axisCount:"<<axisCount;
    //    qDebug()<<"Connection Status: "<<actuator->isConnected();
    //    qDebug()<<"Actuator is homed:"<<actuator->isHomed(axisInd);
    //    double setLastPos;
    //    char filename[]="savedPos.txt";
    //    std::ifstream infile(filename);
    //    std::string lastSavedPos;
    //    if(infile.good()){
    //        getline(infile, lastSavedPos);
    //        setLastPos = std::stod(lastSavedPos);
    //        actuator->setPosition(setLastPos, axisInd);
    //        infile.close();
    //    } else {
    //        std::ofstream infile("savedPos.txt");
    //        actuator->setPosition(0.00, axisInd);
    //        infile << "0.00";
    //        infile.close();
    //    }
    // qDebug()<<"Set to 1/16 Multistep"<<actuator->setGPIO(26, 0);
    qDebug() << "Actuator is homed:" << actuator->isHomed(axisInd);
    count = 0;
    vol_avg = 0;
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
    // std::cout << "Disconnecting Actuator" << std::endl;
    qDebug() << "Disconnecting Actuator\n";
    if (!actuator->disconnect())
    {
        // std::cout << "ERROR: Could not disconnect from actuator" << std::endl;
        qDebug() << "ERROR: Could not disconnect from actuator\n";
    }
    delete actuator;
    actuator = nullptr;
    // std::cout << "Actuator disconnected" << std::endl;
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
                    Sleep(1000);
                    MainWindow::on_pushButton_Connect_clicked();
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
            Sleep(200);
            on_fileinput_clicked();
        }

    }
}

