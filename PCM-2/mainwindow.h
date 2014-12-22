#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ada2.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void newSampleData(QVector<double> data);


private:
    Ui::MainWindow *ui;
    ADA2Device a;
    QVector<double> tick;
};

#endif // MAINWINDOW_H
