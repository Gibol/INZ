#ifndef DIDACTICSWIDGET_H
#define DIDACTICSWIDGET_H

#include <QWidget>

namespace Ui {
class DidacticsWidget;
}

class DidacticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DidacticsWidget(QWidget *parent = 0);
    ~DidacticsWidget();

private:
    Ui::DidacticsWidget *ui;
};

#endif // DIDACTICSWIDGET_H
