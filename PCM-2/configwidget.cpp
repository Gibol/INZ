#include "configwidget.h"
#include "ui_configwidget.h"

ConfigWidget::ConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigWidget)
{
    ui->setupUi(this);
    currentSettings = ADA2Device::initSettingsStructure();
    setWindowTitle("ADA-2 Device Configuration");
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
    currentSettings.wordLenght = (ADA2Device::WordLenght) index;
    if(currentSettings.wordLenght == ADA2Device::Word8bits &&
            (ui->comboBoxError->currentIndex() >= (int)ADA2Device::Bit8 ) && ui->comboBoxError->currentIndex() <= (int)ADA2Device::Bit11)
    {
        ui->comboBoxError->setCurrentIndex((int)ADA2Device::BitNone);
    }
}

void ConfigWidget::on_comboBoxCompression_currentIndexChanged(int index)
{
    currentSettings.compressionType = (ADA2Device::CompressionType) index;
    if(currentSettings.compressionType != ADA2Device::CompressionNone)
    {
        ui->comboBoxWordLen->setCurrentIndex(0);
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
    if(currentSettings.wordLenght == ADA2Device::Word8bits && (index >= (int)ADA2Device::Bit8 ) && index <= (int)ADA2Device::Bit11)
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


