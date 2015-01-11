#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("PCM-2 v1.1");
    setWindowIcon(QIcon(":/icon"));
    
    
    /*Setup toolbar */
    saveGraphButton = new QToolButton(ui->mainToolBar);
    saveDataButton = new QToolButton(ui->mainToolBar);
    playButton = new QToolButton(ui->mainToolBar);
    stopButton = new QToolButton(ui->mainToolBar);
    settingsButton = new QToolButton(ui->mainToolBar);
    saveGraphButton->setIcon(QIcon(":/save"));
    saveDataButton->setIcon(QIcon(":/save"));
    playButton->setIcon(QIcon(":/play"));
    stopButton->setIcon(QIcon(":/stop"));
    settingsButton->setIcon(QIcon(":/settings"));
    saveGraphButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    saveDataButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    playButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    stopButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    settingsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    saveGraphButton->setText(tr("Save graph"));
    saveDataButton->setText(tr("Save data"));
    playButton->setText(tr("Start conversion"));
    stopButton->setText(tr("Stop conversion"));
    settingsButton->setText(tr("Settings"));
    ui->mainToolBar->addWidget(settingsButton);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget(playButton);
    ui->mainToolBar->addWidget(stopButton);
    ui->mainToolBar->addSeparator();
    
    statusIndicator = new StatusIndicator(ui->mainToolBar);
    statusIndicator->setTitle(tr("Status"));
    ui->mainToolBar->addWidget(statusIndicator);
    statusIndicator->setDiode(StatusIndicator::Red, true);
    statusIndicator->setDescription(tr("Unconfigured"));
    
    connectionIndicator = new StatusIndicator(ui->mainToolBar);
    connectionIndicator->setTitle(tr("Link"));
    ui->mainToolBar->addWidget(connectionIndicator);
    connectionIndicator->setDiode(StatusIndicator::Red, true);
    connectionIndicator->setDescription(tr("Disconnected"));
    
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget(saveGraphButton);
    ui->mainToolBar->addWidget(saveDataButton);
    
    /* Start new thread and create the device instance */
    communicationsThread.start();
    adaDevice = new ADA2Device(&communicationsThread);
    
    
    /* setup plot widget */
    ui->plot->xAxis->setVisible(false);
    ui->plot->yAxis->setVisible(false);
    ui->plot->setBackground(QIcon(":/icon").pixmap(QSize(709,498)));
    ui->plot->setBackgroundScaledMode(Qt::IgnoreAspectRatio);
    for(double d = 0.0; d < 500.0; d+=1.0) tick.append(d);
    maxRange = QPair<double,double>(2048.0, 400.0);
    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    /* additional graph for marking trigger range */
    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->graph(1)->setPen(QPen(Qt::magenta));
    ui->plot->graph(2)->setPen(QPen(Qt::magenta));
    ui->plot->graph(1)->setBrush(QBrush(QColor(240, 255, 200,100)));
    ui->plot->graph(1)->setChannelFillGraph(ui->plot->graph(2));
    
    /* set labels font size */
    QFont font = ui->plot->font();
    font.setPointSize(12);
    ui->plot->yAxis->setLabelFont(font);
    ui->plot->xAxis->setLabelFont(font);
    
    /* give the axes some labels */
    ui->plot->xAxis->setLabel(tr("Sample number"));
    ui->plot->yAxis->setLabel(tr("Sample value"));
    
    /* set axes ranges*/
    ui->plot->yAxis->setRange(-maxRange.first, maxRange.first);
    ui->plot->xAxis->setRange(0.0, maxRange.second);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iSelectItems);
    ui->plot->yAxis->grid()->setSubGridVisible(true);
    ui->plot->xAxis->grid()->setSubGridVisible(true);
    
    /* setup markers */
    marker1 = new QCPItemTracer(ui->plot);
    marker2 = new QCPItemTracer(ui->plot);
    marker1->setGraph(ui->plot->graph(0));
    marker2->setGraph(ui->plot->graph(0));
    marker1->setInterpolating(true);
    marker2->setInterpolating(true);
    marker1->setPen(QPen(Qt::red));
    marker2->setPen(QPen(Qt::green));
    marker1->setSelectable(true);
    marker2->setSelectable(true);
    marker1->setVisible(false);
    marker2->setVisible(false);
    
    /* setup connections */
    connect(adaDevice, SIGNAL(newSampleData(QVector<double>)), this, SLOT(newSampleData(QVector<double>)), Qt::QueuedConnection);
    connect(&configWidget, SIGNAL(settingsChanged(ADA2Device::ADASettings)), adaDevice, SLOT(newSettings(ADA2Device::ADASettings)), Qt::QueuedConnection);
    connect(this, SIGNAL(startStopConversionRequest(ADA2Device::StartStopCommand)), adaDevice, SLOT(sendStartStopCommand(ADA2Device::StartStopCommand)), Qt::QueuedConnection);
    connect(adaDevice, SIGNAL(message(QString)), this, SLOT(displayMessage(QString)), Qt::QueuedConnection);
    connect(adaDevice, SIGNAL(connectionStatusChanged(ADA2Device::ConnectionStatus)), this, SLOT(connectionStatusChanged(ADA2Device::ConnectionStatus)), Qt::QueuedConnection);
    connect(adaDevice, SIGNAL(deviceStatusChanged(ADA2Device::DeviceStatus)), this, SLOT(deviceStatusChanged(ADA2Device::DeviceStatus)), Qt::QueuedConnection);
    connect(&configWidget, SIGNAL(settingsChanged(ADA2Device::ADASettings)), this, SLOT(newConfig(ADA2Device::ADASettings)));
    connect(marker1, SIGNAL(selectionChanged(bool)), this, SLOT(setFocus(bool)));
    connect(marker2, SIGNAL(selectionChanged(bool)), this, SLOT(setFocus(bool)));
    connect(saveDataButton, SIGNAL(clicked()), this, SLOT(saveDataClicked()));
    connect(playButton, SIGNAL(clicked()), this, SLOT(startConversionClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopConversionClicked()));
    connect(saveGraphButton, SIGNAL(clicked()), this, SLOT(saveGraphClicked()));
    connect(settingsButton, SIGNAL(clicked()), this, SLOT(settingsClicked()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->actionShow_compression_characteristics, SIGNAL(triggered()), this, SLOT(showDidactics()));
    
    /* start main gui timer */
    startTimer(1000);
}

MainWindow::~MainWindow()
{
    communicationsThread.exit();
    configWidget.close();
    aboutWidget.close();
    didacticsWidget.close();
    delete adaDevice;
    delete ui;
    delete marker1;
    delete marker2;
    delete saveGraphButton;
    delete saveDataButton;
    delete playButton;
    delete stopButton;
    delete settingsButton;
    delete connectionIndicator;
    delete statusIndicator;
}

void MainWindow::newSampleData(QVector<double> data)
{
    
    if(ui->radioButtonVms->isChecked())
    {
        /* sample conversion */
        int sampleMaxValue = 4096;
        if(currentDevSettings.compressionType == ADA2Device::None)
        {
            sampleMaxValue = (1 << currentDevSettings.wordLenght);
        }
        for(int i = 0; i < data.count(); i++)
        {
            data[i] *= 10.0/sampleMaxValue;
        }
        
    }
    
    qint16 triggerPoint = findTriggerPoint(data, ui->doubleSpinBoxLevel->value(), ui->doubleSpinBoxPrecision->value(),ui->radioButtonRising->isChecked());
    if(triggerPoint > -1)
        ui->plot->graph(0)->setData(tick, data.mid(triggerPoint, 500));
    else if(!ui->checkBoxPlotOnlyWhenTriggered->isChecked())
        ui->plot->graph(0)->setData(tick, data);
    
    updateMarkers();
    ui->plot->replot();
    statusIndicator->setDiode(StatusIndicator::Green, true);
}

void MainWindow::startStopConv()
{
    static bool b = false;
    if(b) emit startStopConversionRequest(ADA2Device::Stop);
    else startStopConversionRequest(ADA2Device::Start);
    b = !b;
}

qint16 MainWindow::findTriggerPoint(const QVector<double> &data, double triggerLevel, double triggerPrecision, bool risingFalling)
{
    if(data.count() < 10) return -1; //pointless to search for trigger point in smaller vectors
    
    for(int i = 0; i < data.count()-1; i++)
    {
        if(data.at(i) < triggerLevel+triggerPrecision && data.at(i) > triggerLevel-triggerPrecision)
        {
            if( (risingFalling && (data.at(i+1) > data.at(i))) || (!risingFalling && (data.at(i+1) < data.at(i))) )
            {
                return i;
            }
        }
    }
    return -1;
}

void MainWindow::on_doubleSpinBoxLevel_valueChanged(double arg1)
{
    QVector<double> temp;
    for(double d = 0.0; d < 500.0; d+=1.0) temp.append(arg1+ui->doubleSpinBoxPrecision->value());
    ui->plot->graph(1)->setData(tick, temp);
    temp.clear();
    for(double d = 0.0; d < 500.0; d+=1.0) temp.append(arg1-ui->doubleSpinBoxPrecision->value());
    ui->plot->graph(2)->setData(tick, temp);
    ui->plot->replot();
}

void MainWindow::on_doubleSpinBoxPrecision_valueChanged(double arg1)
{
    QVector<double> temp;
    for(double d = 0.0; d < 500.0; d+=1.0) temp.append(ui->doubleSpinBoxLevel->value()+arg1);
    ui->plot->graph(1)->setData(tick, temp);
    temp.clear();
    for(double d = 0.0; d < 500.0; d+=1.0) temp.append(ui->doubleSpinBoxLevel->value()-arg1);
    ui->plot->graph(2)->setData(tick, temp);
    ui->plot->replot();
}

void MainWindow::displayMessage(QString msg)
{
    userMessages.enqueue(msg);
    if(userMessages.count() > 5) userMessages.dequeue();
}

void MainWindow::startConversionClicked()
{
    ui->plot->xAxis->setVisible(true);
    ui->plot->yAxis->setVisible(true);
    ui->plot->setBackground(QPixmap());
    
    if(adaDevice->getConnectionStatus() == ADA2Device::Connected && adaDevice->getDeviceStatus() == ADA2Device::Configured)
    {
        emit startStopConversionRequest(ADA2Device::Start);
    }
    else if(adaDevice->getConnectionStatus() == ADA2Device::Connected)
    {
        configWidget.show();
        configWidget.raise();
    }
    else //disconnected
    {
        QMessageBox::critical(0, tr("Error."), tr("ADA-2 Device is not connected!"), QMessageBox::Ok);
    }
}

void MainWindow::stopConversionClicked()
{
    emit startStopConversionRequest(ADA2Device::Stop);
}

void MainWindow::settingsClicked()
{
    configWidget.show();
    configWidget.raise();
}

void MainWindow::saveGraphClicked()
{
    
    bool okFlag = false;
    int width = QInputDialog::getInt(0, tr("Save plot..."), tr("Image Width"), 1000, 100, 10000, 100, &okFlag);
    if(!okFlag) return;
    
    int height = QInputDialog::getInt(0, tr("Save plot..."), tr("Image Height"), 1000, 100, 10000, 100, &okFlag);
    if(!okFlag) return;
    
    
    QString filter;
    filter.append("Portable Network Graphics (*.png);;");
    filter.append("JPEG (*.jpg);;");
    filter.append("Bitmap (*.bmp);;");
    filter.append("Adobe PDF (*.pdf)");
    QString path = QFileDialog::getSaveFileName(0, tr("Save plot..."), QDir::currentPath(), filter );
    
    QVector<bool> markers;
    markers.append(ui->checkBoxMarker1->isChecked());
    markers.append(ui->checkBoxMarker2->isChecked());
    markers.append(ui->checkBoxShowMarker->isChecked());
    ui->checkBoxMarker1->setChecked(false);
    ui->checkBoxMarker2->setChecked(false);
    ui->checkBoxShowMarker->setChecked(false);
    if(path.endsWith("pdf"))
    {
        okFlag = ui->plot->savePdf(path, true, width, height, "PCM-2", "PCM-2 Graph");
    }
    else
    {
        okFlag = ui->plot->saveRastered(path, width, height, 1.0, 0, 100);
    }
    
    ui->checkBoxMarker1->setChecked(markers.at(0));
    ui->checkBoxMarker2->setChecked(markers.at(1));
    ui->checkBoxShowMarker->setChecked(markers.at(2));
    
    if(!okFlag) QMessageBox::critical(0, tr("Error"), tr("Error saving file!"));
}

void MainWindow::saveDataClicked()
{
    /* cpy current data */
    QList<QCPData> graphData = ui->plot->graph(0)->data()->values();
    
    QString filter;
    filter.append("Comma Separated Values file (*.csv)");
    QString path = QFileDialog::getSaveFileName(0, tr("Save data..."), QDir::currentPath(), filter );
    
    if(path.isEmpty())
    {
        QMessageBox::critical(0, tr("Error"), tr("Error saving file!"));
        return;
    }
    
    QFile f(path);
    if(!f.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QMessageBox::critical(0, tr("Error"), tr("Error saving file!"));
    }
    
    QTextStream s(&f);
    s<<"PCM-2 data file. "<<QDateTime::currentDateTime().toString()<<endl;
    for(int i = 0; i < graphData.count(); i++)
    {
        s<<QString("%1 %2,").arg(graphData.at(i).key).arg(graphData.at(i).value)<<endl;
    }
    
    f.close();
}

void MainWindow::deviceStatusChanged(ADA2Device::DeviceStatus status)
{   
    static ADA2Device::DeviceStatus previousStatus = ADA2Device::Idle;
    
    if(status == ADA2Device::Idle)
    {
        statusIndicator->setDiode(StatusIndicator::Red, true);
        statusIndicator->setDescription(tr("Idle"));
    }
    else if(status == ADA2Device::Configured)
    {
        statusIndicator->setDiode(StatusIndicator::Yellow, true);
        statusIndicator->setDescription(tr("Configured"));
        if(previousStatus != ADA2Device::Configured && !configWidget.isHidden()) configWidget.hide();
    }
    else
    {
        statusIndicator->setDiode(StatusIndicator::Green, false);
        statusIndicator->setDescription(tr("Busy"));
    }
    
    previousStatus = status;
}

void MainWindow::connectionStatusChanged(ADA2Device::ConnectionStatus status)
{
    if(status == ADA2Device::Disconnected)
    {
        connectionIndicator->setDiode(StatusIndicator::Red, true);
        connectionIndicator->setDescription(tr("Disconnected"));
    }
    else if(status == ADA2Device::Waiting)
    {
        connectionIndicator->setDiode(StatusIndicator::Yellow, true);
        connectionIndicator->setDescription(tr("Waiting for response"));
    }
    else
    {
        connectionIndicator->setDiode(StatusIndicator::Green, true);
        connectionIndicator->setDescription(tr("Connected"));
    }
    
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if(!userMessages.isEmpty()) ui->statusBar->showMessage(userMessages.dequeue(), 5000);
    //connectionIndicator->setDiode(StatusIndicator::Green, false);
    //statusIndicator->setDiode(StatusIndicator::Green, false);
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    bool moveLeft = false, moveRight = false;
    int stepValue = 1;
    
    if(event->key() == Qt::Key_Left)
    {
        moveLeft = true;
    }
    else if(event->key() == Qt::Key_Right)
    {
        moveRight = true;
    }
    else if(event->key() == Qt::Key_Down)
    {
        moveLeft = true;
        stepValue =10;
    }
    else if(event->key() == Qt::Key_Up)
    {
        moveRight = true;
        stepValue = 10;
    }
    
    if(marker1->selected())
    {
        QList<double> keys = ui->plot->graph(0)->data()->keys();
        int currentKeyIndex = keys.indexOf(marker1->graphKey());
        if(moveLeft)
        {
            if(currentKeyIndex - stepValue >= 0) marker1->setGraphKey(keys.at(currentKeyIndex-stepValue));
        }
        else if(moveRight)
        {
            if(currentKeyIndex + stepValue < keys.size()) marker1->setGraphKey(keys.at(currentKeyIndex+stepValue));
        }
        
    }
    else if(marker2->selected())
    {
        QList<double> keys = ui->plot->graph(0)->data()->keys();
        int currentKeyIndex = keys.indexOf(marker2->graphKey());
        if(moveLeft)
        {
            if(currentKeyIndex - stepValue >= 0) marker2->setGraphKey(keys.at(currentKeyIndex-stepValue));
        }
        else if(moveRight)
        {
            if(currentKeyIndex + stepValue < keys.size()) marker2->setGraphKey(keys.at(currentKeyIndex+stepValue));
        }
        
    }
    updateMarkers();
    ui->plot->replot();
    event->accept();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    QApplication::quit();
}

void MainWindow::on_horizontalSliderVerticalZoom_valueChanged(int value)
{
    ui->plot->yAxis->setRange(-maxRange.first + (maxRange.first/100*value), maxRange.first - (maxRange.first/100*value) );
    ui->plot->replot();
}

void MainWindow::on_horizontalSliderHorizontalZoom_valueChanged(int value)
{
    ui->plot->xAxis->setRange(0.0, maxRange.second - (maxRange.second/100*value));
    ui->plot->replot();
}

void MainWindow::on_radioButtonVms_toggled(bool checked)
{
    tick.clear();
    double sampleTime;
    
    if(!checked)
    {
        sampleTime = 0.001;
        if(currentDevSettings.compressionType == ADA2Device::None)
        {
            maxRange.first = (1 << (currentDevSettings.wordLenght -1) );
        }
        else
        {
            maxRange.first = 2048;
        }
    }
    else
    {
        maxRange.first = 10.0/2;
        
        switch(currentDevSettings.samplingFrequency)
        {
        case ADA2Device::F8KHZ:
            sampleTime = 1.0/8000;
            break;
            
        case ADA2Device::F16KHZ:
            sampleTime = 1.0/16000;
            break;
            
        case ADA2Device::F32KHZ:
            sampleTime = 1.0/32000;
            break;
            
        case ADA2Device::F44_1KHZ:
            sampleTime = 1.0/44100;
            break;
            
        case ADA2Device::F22_05KHZ:
            sampleTime = 1.0 / 22050;
            break;
            
        case ADA2Device::F11_025KHZ:
            sampleTime = 1.0 / 11025;
            break;
            
        default:
            sampleTime = 0;
            break;
        }
    }
    maxRange.second = sampleTime*400*1000;
    for(double d = 0.0; d < 500.0; d+=1.0) tick.append(sampleTime*d*1000);
    
    ui->plot->xAxis->setRange(0.0, maxRange.second);
    ui->plot->yAxis->setRange(-maxRange.first, maxRange.first);
    
    double currentTriggerLevel = ui->doubleSpinBoxLevel->value(), currentTriggerPrecision = ui->doubleSpinBoxPrecision->value();
    
    ui->doubleSpinBoxLevel->setMinimum(-maxRange.first);
    ui->doubleSpinBoxLevel->setMaximum(maxRange.first);
    ui->doubleSpinBoxPrecision->setMaximum(maxRange.first / 10.0);
    
    int sampleMaxValue = 4096;
    if(currentDevSettings.compressionType == ADA2Device::None)
    {
        sampleMaxValue = (1 << currentDevSettings.wordLenght);
    }

    
    if(checked)
    {
        ui->doubleSpinBoxLevel->setValue(currentTriggerLevel*10/sampleMaxValue);
        ui->doubleSpinBoxPrecision->setValue(currentTriggerPrecision*10/sampleMaxValue);
        ui->doubleSpinBoxLevel->setSingleStep(0.01);
        ui->doubleSpinBoxPrecision->setSingleStep(0.01);
        ui->plot->xAxis->setLabel(tr("Time [ms]"));
        ui->plot->yAxis->setLabel(tr("Amplitude [V]"));
    }
    else
    {
        ui->doubleSpinBoxLevel->setValue(currentTriggerLevel/10*sampleMaxValue);
        ui->doubleSpinBoxPrecision->setValue(currentTriggerPrecision/10*sampleMaxValue);
        ui->doubleSpinBoxLevel->setSingleStep(1.0);
        ui->doubleSpinBoxPrecision->setSingleStep(1.0);
        ui->plot->xAxis->setLabel(tr("Sample number"));
        ui->plot->yAxis->setLabel(tr("Sample value"));
    }
    
    if(!ui->plot->graph(0)->data()->isEmpty())
    {
        marker1->setGraphKey(ui->plot->graph(0)->data()->keys().first());
        marker2->setGraphKey(ui->plot->graph(0)->data()->keys().first());
    }
    
    ui->horizontalSliderHorizontalZoom->setValue(0);
    ui->horizontalSliderVerticalZoom->setValue(0);
    
    ui->plot->replot();
}

void MainWindow::on_checkBoxShowMarker_toggled(bool checked)
{
    ui->plot->graph(1)->setVisible(checked);
    ui->plot->graph(2)->setVisible(checked);
    ui->plot->replot();
}

void MainWindow::setFocus(bool)
{
    QWidget::setFocus();
}

void MainWindow::on_checkBoxMarker1_toggled(bool checked)
{
    marker1->setVisible(checked);
    ui->plot->replot();
}

void MainWindow::on_checkBoxMarker2_toggled(bool checked)
{
    marker2->setVisible(checked);
    ui->plot->replot();
}


void MainWindow::on_radioButtonLine_toggled(bool checked)
{
    if(checked) ui->plot->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->plot->replot();
}

void MainWindow::on_radioButtonStep_toggled(bool checked)
{
    if(checked) ui->plot->graph(0)->setLineStyle(QCPGraph::lsStepLeft);
    ui->plot->replot();
}

void MainWindow::on_radioButtonImpulse_toggled(bool checked)
{
    if(checked) ui->plot->graph(0)->setLineStyle(QCPGraph::lsImpulse);
    ui->plot->replot();
}

void MainWindow::on_radioButtonPoint_toggled(bool checked)
{
    if(checked) ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    else ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->plot->replot();
}

void MainWindow::newConfig(ADA2Device::ADASettings settings)
{
    currentDevSettings = settings;
    statusIndicator->setDiode(StatusIndicator::Red, true);
    statusIndicator->setDescription(tr("Idle"));
    
    /* invoke this to change axes scale etc */
    on_radioButtonVms_toggled(ui->radioButtonVms->isChecked());
}

void MainWindow::showAbout()
{
    aboutWidget.show();
    aboutWidget.raise();
}

void MainWindow::showDidactics()
{
    didacticsWidget.show();
    didacticsWidget.raise();
}

void MainWindow::updateMarkers()
{
    if(marker1->visible())
    {
        ui->label1XValue->setText(QString::number(marker1->graphKey())+ ( (ui->radioButtonSamples->isChecked()) ? "" : " ms") );
        ui->label1YValue->setText(QString::number(ui->plot->graph(0)->data()->value(marker1->graphKey()).value) + ( (ui->radioButtonSamples->isChecked()) ? "" : " V"));
    }
    else
    {
        ui->label1XValue->setText("");
        ui->label1YValue->setText("");
    }
    if(marker2->visible())
    {
        ui->label2XValue->setText(QString::number(marker2->graphKey())+ ( (ui->radioButtonSamples->isChecked()) ? "" : " ms") );
        ui->label2YValue->setText(QString::number(ui->plot->graph(0)->data()->value(marker2->graphKey()).value) + ( (ui->radioButtonSamples->isChecked()) ? "" : " V"));
    }
    else
    {
        ui->label2XValue->setText("");
        ui->label2YValue->setText("");
    }
    if(marker1->visible() && marker2->visible() && ui->radioButtonVms->isChecked())
    {
        ui->labelFrequency->setText(QString::number(abs(1.0 / (marker1->graphKey() - marker2->graphKey() ) * 1000  )) + " Hz");
    }
    else
    {
        ui->labelFrequency->setText("");
    }
}


