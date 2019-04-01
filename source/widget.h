#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "server.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
private slots:
    void ShowMsg(const string);

    void on_browseButton_clicked();

    void on_startButton_clicked(bool checked);

    //void finished(bool);

private:
    Ui::Widget *ui;
    Server *Srv;
};

#endif // WIDGET_H
