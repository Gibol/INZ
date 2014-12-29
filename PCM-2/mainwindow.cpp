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


    for(double d = 0.0; d < 1000.0; d+=1.0) tick.append(d);


    ui->centralWidget->addGraph();
    //ui->centralWidget->graph(0)->setData(tick, data);
    ui->centralWidget->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph

    ui->centralWidget->addGraph();
    //ui->centralWidget->graph(0)->setData(tick, data);
    ui->centralWidget->graph(1)->setPen(QPen(Qt::magenta)); // line color blue for first graph

    // give the axes some labels:
    ui->centralWidget->xAxis->setLabel("x");
    ui->centralWidget->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->centralWidget->yAxis->setRange(0.0, 4095.0);
    ui->centralWidget->xAxis->setRange(0.0, 500.0);

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

    ui->centralWidget->graph(0)->setData(tick, data.mid(0,500));
    ui->centralWidget->graph(1)->setData(tick, data.mid(500,500));
    ui->centralWidget->replot();
}

void MainWindow::startStopConv()
{
    static bool b = false;
    if(b) emit startStopConversionRequest(ADA2Device::Stop);
    else startStopConversionRequest(ADA2Device::Start);
    b = !b;
}
