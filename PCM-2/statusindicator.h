#ifndef STATUSINDICATOR_H
#define STATUSINDICATOR_H

#include <QWidget>
#include <QPainter>

namespace Ui {
class StatusIndicator;
}

class StatusIndicator : public QWidget
{
    Q_OBJECT

public:
    typedef enum { Green = Qt::green, Yellow = Qt::yellow, Red = Qt::red } DiodeColor;

    explicit StatusIndicator(QWidget *parent = 0);
    ~StatusIndicator();


public slots:
    void setDiode(StatusIndicator::DiodeColor color, bool lightOn);
    void setDescription(QString description);
    void setTitle(QString title);

private:
    Ui::StatusIndicator *ui;
};

#endif // STATUSINDICATOR_H
