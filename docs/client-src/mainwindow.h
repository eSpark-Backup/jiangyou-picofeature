#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "actuator_lib.h"
#include <QTimer>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_Home_clicked();

    void on_pushButton_MoveAbs_clicked();

    void on_pushButton_Connect_clicked();

    void on_pushButton_MoveRel_clicked();

    void on_pushButton_Disconnect_clicked();

    void on_pushButton_Stop_clicked();

    void mytimerhandler();

    void on_onBtn_clicked();

    void on_offBtn_clicked();

    void on_setPosBtn_clicked();
    
    void on_reset_clicked();

    void on_fileinput_clicked();


private:
    Ui::MainWindow *ui;
   Actuator *actuator;
     QTimer mytimer;
     int axisInd;
     int deviceID;

     double vol_avg;

     int count;

};
#endif // MAINWINDOW_H
