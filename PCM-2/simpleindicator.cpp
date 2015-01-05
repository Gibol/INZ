#include "simpleindicator.h"

SimpleIndicator::SimpleIndicator(QWidget *parent) :
    QWidget(parent)
{
    _baseColor = QColor(Qt::red);
}

void SimpleIndicator::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QPointF center(size().width()/2, size().width()/2);
    qreal radius = size().width()/2 - 1;
    QRadialGradient g(center, radius);
    g.setColorAt(0.0, _baseColor);
    g.setColorAt(1.0, _baseColor.darker());
    p.setBrush(g);
    QPen pen;
    pen.setColor(Qt::gray);
    pen.setWidth(2);
    p.setPen(pen);
    p.drawEllipse(center, radius,radius);
    p.end();
    event->accept();

}

void SimpleIndicator::setColor(QColor baseColor)
{
    _baseColor = baseColor;
    repaint();
}
