#include "statusindicator.h"
#include "ui_statusindicator.h"

StatusIndicator::StatusIndicator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusIndicator)
{
    ui->setupUi(this);
    ui->widget->setColor(QColor(Qt::darkGreen));
}

StatusIndicator::~StatusIndicator()
{
    delete ui;
}

void StatusIndicator::setDiode(StatusIndicator::DiodeColor color, bool lightOn)
{
   QColor col((Qt::GlobalColor) color);
   if(!lightOn) col = col.darker();

   ui->widget->setColor(col);
}

void StatusIndicator::setDescription(QString description)
{
    ui->label->setText(description);
}

void StatusIndicator::setTitle(QString title)
{
    ui->label_2->setText(title);
}
