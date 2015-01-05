#ifndef SIMPLEINDICATOR_H
#define SIMPLEINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class SimpleIndicator : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleIndicator(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void setColor(QColor baseColor);

private:
    int heightForWidth(int w) { return w; }
    QColor _baseColor;
};

#endif // SIMPLEINDICATOR_H
