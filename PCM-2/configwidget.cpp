#include "configwidget.h"
#include "ui_configwidget.h"

ConfigWidget::ConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigWidget)
{
    ui->setupUi(this);
    currentSettings = ADA2Device::initSettingsStructure();
    setWindowTitle(tr("ADA-2 Device Configuration"));
}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

void ConfigWidget::on_comboBoxSource_currentIndexChanged(int index)
{
    currentSettings.signalSource = (ADA2Device::SignalSource) index;
}

void ConfigWidget::on_comboBoxWordLen_currentIndexChanged(int index)
{
    currentSettings.wordLenght = (ADA2Device::WordLenght) (index*2 + 4);

    ui->comboBoxError->setCurrentIndex((int)ADA2Device::BitNone);

}

void ConfigWidget::on_comboBoxCompression_currentIndexChanged(int index)
{
    currentSettings.compressionType = (ADA2Device::CompressionType) index;
    if(currentSettings.compressionType == ADA2Device::MuAnalog || currentSettings.compressionType == ADA2Device::AAnalog)
    {
        ui->doubleSpinBoxCompressionParam->setEnabled(true);
    }
    else
    {
        ui->doubleSpinBoxCompressionParam->setEnabled(false);
    }

    if(currentSettings.compressionType != ADA2Device::None)
    {
        ui->comboBoxWordLen->setCurrentIndex(2);
        ui->comboBoxWordLen->setDisabled(true);
    }
    else
    {
        ui->comboBoxWordLen->setDisabled(false);
    }
}

void ConfigWidget::on_comboBoxFp_currentIndexChanged(int index)
{
    currentSettings.samplingFrequency = (ADA2Device::SampligFrequency) index;
}

void ConfigWidget::on_comboBoxError_currentIndexChanged(int index)
{
    if(currentSettings.wordLenght == ADA2Device::Word4bits && (index >= (int)ADA2Device::Bit4 ) && index <= (int)ADA2Device::Bit11)
    {
        ui->comboBoxError->setCurrentIndex((int)ADA2Device::BitNone);
    }
    else if(currentSettings.wordLenght == ADA2Device::Word6bits && (index >= (int)ADA2Device::Bit6 ) && index <= (int)ADA2Device::Bit11)
    {
        ui->comboBoxError->setCurrentIndex((int)ADA2Device::BitNone);
    }
    else if(currentSettings.wordLenght == ADA2Device::Word8bits && (index >= (int)ADA2Device::Bit8 ) && index <= (int)ADA2Device::Bit11)
    {
        ui->comboBoxError->setCurrentIndex((int)ADA2Device::BitNone);
    }
    else if(currentSettings.wordLenght == ADA2Device::Word10bits && (index >= (int)ADA2Device::Bit10 ) && index <= (int)ADA2Device::Bit11)
    {
        ui->comboBoxError->setCurrentIndex((int)ADA2Device::BitNone);
    }
    currentSettings.bitError = (ADA2Device::BitError) index;
}

void ConfigWidget::on_comboBoxOutput_currentIndexChanged(int index)
{
    currentSettings.signalOutput = (ADA2Device::SignalOutput) index;
}

void ConfigWidget::on_pushButton_clicked()
{
    emit settingsChanged(currentSettings);
}



void ConfigWidget::on_doubleSpinBoxCompressionParam_valueChanged(double arg1)
{
    currentSettings.analogCompressionParam = arg1*10;
}
