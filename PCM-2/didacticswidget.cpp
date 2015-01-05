#include "didacticswidget.h"
#include "ui_didacticswidget.h"

DidacticsWidget::DidacticsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DidacticsWidget)
{
    ui->setupUi(this);
    setWindowTitle("Didactic informations");
}

DidacticsWidget::~DidacticsWidget()
{
    delete ui;
}
