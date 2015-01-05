#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QQueue>
#include <QString>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include "ada2.h"
#include "configwidget.h"
#include "statusindicator.h"
#include "qcustomplot.h"
#include "didacticswidget.h"
#include "aboutwidget.h"

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
    void displayMessage(QString msg);
    void startConversionClicked();
    void stopConversionClicked();
    void settingsClicked();
    void saveGraphClicked();
    void saveDataClicked();
    void deviceStatusChanged(ADA2Device::DeviceStatus status);
    void connectionStatusChanged(ADA2Device::ConnectionStatus status);
    void on_horizontalSliderVerticalZoom_valueChanged(int value);
    void on_horizontalSliderHorizontalZoom_valueChanged(int value);
    void on_radioButtonVms_toggled(bool checked);
    void on_checkBoxShowMarker_toggled(bool checked);
    void setFocus(bool);
    void on_checkBoxMarker1_toggled(bool checked);
    void on_checkBoxMarker2_toggled(bool checked);
    void on_radioButtonLine_toggled(bool checked);
    void on_radioButtonStep_toggled(bool checked);
    void on_radioButtonImpulse_toggled(bool checked);
    void on_radioButtonPoint_toggled(bool checked);
    void newConfig(ADA2Device::ADASettings settings);
    void showAbout();
    void showDidactics();

signals:
    void startStopConversionRequest(ADA2Device::StartStopCommand);

protected:
    virtual void timerEvent(QTimerEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    QCPItemTracer *marker1;
    QCPItemTracer *marker2;
    QToolButton *saveGraphButton;
    QToolButton *saveDataButton;
    QToolButton *playButton;
    QToolButton *stopButton;
    QToolButton *settingsButton;
    ADA2Device *adaDevice;
    StatusIndicator *connectionIndicator;
    StatusIndicator *statusIndicator;

    ConfigWidget configWidget;
    DidacticsWidget didacticsWidget;
    AboutWidget aboutWidget;
    QVector<double> tick;
    QThread communicationsThread;
    quint16 findTriggerPoint(QVector<double> const &data, double triggerLevel, double triggerPrecision, double step, bool risingFalling);
    QQueue<QString> userMessages;
    QPair<double,double> maxRange;
    ADA2Device::ADASettings currentDevSettings;


};


#endif // MAINWINDOW_H
