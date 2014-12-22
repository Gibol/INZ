#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


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

    connect(&a, SIGNAL(newSampleData(QVector<double>)), this, SLOT(newSampleData(QVector<double>)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newSampleData(QVector<double> data)
{

    ui->centralWidget->graph(0)->setData(tick, data.mid(0,500));
    ui->centralWidget->graph(1)->setData(tick, data.mid(500,500));
    ui->centralWidget->replot();
}
