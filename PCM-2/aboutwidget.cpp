#include "aboutwidget.h"
#include "ui_aboutwidget.h"

AboutWidget::AboutWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutWidget)
{
    ui->setupUi(this);
    setWindowTitle("About PCM-2");
}

AboutWidget::~AboutWidget()
{
    delete ui;
}

void AboutWidget::on_pushButton_clicked()
{
    QApplication::aboutQt();
}

void AboutWidget::on_pushButton_2_clicked()
{
    QDesktopServices::openUrl(QUrl("mailto:pawel@gibaszek.pl"));
}

void AboutWidget::on_pushButton_3_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Gibol/INZ"));
}
