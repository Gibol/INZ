#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* Start new thread and create the device instance */
    communicationsThread.start();
    adaDevice = new ADA2Device(&communicationsThread);


    for(double d = 0.0; d < 500.0; d+=1.0) tick.append(d);


    ui->plot->addGraph();
    //ui->plot->graph(0)->setData(tick, data);
    ui->plot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph

    ui->plot->addGraph();
    ui->plot->addGraph();
    //ui->plot->graph(0)->setData(tick, data);
    ui->plot->graph(1)->setPen(QPen(Qt::magenta)); // line color blue for first graph
    ui->plot->graph(2)->setPen(QPen(Qt::magenta)); // line color blue for first graph
    ui->plot->graph(1)->setBrush(QBrush(QColor(240, 255, 200,100)));
    ui->plot->graph(1)->setChannelFillGraph(ui->plot->graph(2));


    // give the axes some labels:
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->plot->yAxis->setRange(-2047.0, 2048.0);
    ui->plot->xAxis->setRange(0.0, 500.0);

    ui->plot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);

    connect(adaDevice, SIGNAL(newSampleData(QVector<double>)), this, SLOT(newSampleData(QVector<double>)), Qt::QueuedConnection);
    connect(ui->actionUstawienia_Urz_dzenia, SIGNAL(triggered()), &configWidget, SLOT(show()));
    connect(&configWidget, SIGNAL(settingsChanged(ADA2Device::ADASettings)), adaDevice, SLOT(newSettings(ADA2Device::ADASettings)), Qt::QueuedConnection);
    connect(ui->actionStart_Conversion, SIGNAL(triggered()), this, SLOT(startStopConv()));
    connect(this, SIGNAL(startStopConversionRequest(ADA2Device::StartStopCommand)), adaDevice, SLOT(sendStartStopCommand(ADA2Device::StartStopCommand)), Qt::QueuedConnection);

}

MainWindow::~MainWindow()
{
    communicationsThread.exit();
    delete adaDevice;
    delete ui;
}

void MainWindow::newSampleData(QVector<double> data)
{


    ui->plot->graph(0)->setData(tick, data.mid(findTriggerPoint(data, ui->doubleSpinBoxLevel->value(), ui->doubleSpinBoxPrecision->value(), ui->radioButtonRising->isChecked()), 500));
    //ui->plot->graph(1)->setData(tick, data.mid(500,500));
    ui->plot->replot();
}

void MainWindow::startStopConv()
{
    static bool b = false;
    if(b) emit startStopConversionRequest(ADA2Device::Stop);
    else startStopConversionRequest(ADA2Device::Start);
    b = !b;
}

quint16 MainWindow::findTriggerPoint(const QVector<double> &data, double triggerLevel, double triggerPrecision, bool risingFalling)
{
    if(data.count() < 10) return 0; //pointless to search for trigger point in smaller vectors

    double step = 1.0;
    double currentDeviation = 0.0;
    //qDebug()<<data;
    for(int d = 0; d < (int)(triggerPrecision/step); d++)
    {

        for(int i = 0; i < data.count()-1; i++)
        {
            if(data.at(i) == triggerLevel+currentDeviation || data.at(i) == triggerLevel-currentDeviation)
            {
                //qDebug()<<d<<currentDeviation<<i<<data.at(i)<<((data.at(i+1) > data.at(i))) << ((data.at(i+1) < data.at(i)));
                if( (risingFalling && (data.at(i+1) > data.at(i))) || (!risingFalling && (data.at(i+1) < data.at(i))) )
                {
                    //qDebug()<<"TRIG"<<i<<data.at(i)<<data.at(i+1);
                    return i;
                }
            }
        }
        currentDeviation += step;
    }
    return 0;
}

void MainWindow::on_doubleSpinBoxLevel_valueChanged(double arg1)
{
    QVector<double> temp;
    for(double d = 0.0; d < 500.0; d+=1.0) temp.append(arg1+ui->doubleSpinBoxPrecision->value());
    ui->plot->graph(1)->setData(tick, temp);
    temp.clear();
    for(double d = 0.0; d < 500.0; d+=1.0) temp.append(arg1-ui->doubleSpinBoxPrecision->value());
    ui->plot->graph(2)->setData(tick, temp);
}

void MainWindow::on_doubleSpinBoxPrecision_valueChanged(double arg1)
{
    QVector<double> temp;
    for(double d = 0.0; d < 500.0; d+=1.0) temp.append(ui->doubleSpinBoxLevel->value()+arg1);
    ui->plot->graph(1)->setData(tick, temp);
    temp.clear();
    for(double d = 0.0; d < 500.0; d+=1.0) temp.append(ui->doubleSpinBoxLevel->value()-arg1);
    ui->plot->graph(2)->setData(tick, temp);
}
