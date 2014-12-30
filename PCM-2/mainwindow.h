#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "ada2.h"
#include "configwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void newSampleData(QVector<double> data);
    void startStopConv();

    void on_doubleSpinBoxLevel_valueChanged(double arg1);

    void on_doubleSpinBoxPrecision_valueChanged(double arg1);

signals:
    void startStopConversionRequest(ADA2Device::StartStopCommand);

private:
    Ui::MainWindow *ui;
    ADA2Device *adaDevice;
    ConfigWidget configWidget;
    QVector<double> tick;
    QThread communicationsThread;
    quint16 findTriggerPoint(QVector<double> const &data, double triggerLevel, double triggerPrecision, bool risingFalling);
};

#endif // MAINWINDOW_H
