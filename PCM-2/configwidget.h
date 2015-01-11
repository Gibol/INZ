#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QWidget>
#include "ada2.h"

namespace Ui {
class ConfigWidget;
}

class ConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigWidget(QWidget *parent = 0);
    ~ConfigWidget();

private slots:
    void on_comboBoxSource_currentIndexChanged(int index);

    void on_comboBoxWordLen_currentIndexChanged(int index);

    void on_comboBoxCompression_currentIndexChanged(int index);

    void on_comboBoxFp_currentIndexChanged(int index);

    void on_comboBoxError_currentIndexChanged(int index);

    void on_pushButton_clicked();

    void on_comboBoxOutput_currentIndexChanged(int index);

    void on_doubleSpinBoxCompressionParam_valueChanged(double arg1);

private:
    Ui::ConfigWidget *ui;
    ADA2Device::ADASettings currentSettings;

signals:
    void settingsChanged(ADA2Device::ADASettings);
};

#endif // CONFIGWIDGET_H
