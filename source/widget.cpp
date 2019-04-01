#include "widget.h"
#include "ui_widget.h"
#include <QtWidgets>
#include <QDialog>
#include <QDir>

FILE* logfile;  //全局日志文件

void QStr2str(const QString&,string&);

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("TCP_EXP");
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b"); //IP的正则表达式
    ui->lineEdit_IP->setValidator(new QRegExpValidator(rx,this)); //设置IP输入框

    ui->lineEdit_IP->setText("127.0.0.1");
    ui->lineEdit_Port->setText("80");

}

Widget::~Widget()
{
    delete ui;
}

void Widget::ShowMsg(const string Msg){
    QString QMsg;
    QMsg = QString(QString::fromLocal8Bit(Msg.c_str()));
    ui->textBrowser->insertPlainText(QMsg);
    ui->textBrowser->moveCursor(QTextCursor::End);

}

void Widget::on_browseButton_clicked()
{
    QString directory = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,tr("Find Files"),QDir::currentPath()));

    if(!directory.isEmpty()){
        if(ui->directoryBox->findText(directory)== -1)
            ui->directoryBox->addItem(directory);
        ui->directoryBox->setCurrentIndex(ui->directoryBox->findText(directory));
    }
}

void Widget::on_startButton_clicked(bool checked)
{
    if(checked){//启动服务器
        string root;
        string ip;
        int port;
        QStr2str(ui->directoryBox->currentText(),root);
        QStr2str(ui->lineEdit_IP->text(),ip);
        port = ui->lineEdit_Port->text().toInt();
        logfile = fopen("log.txt","a");
        if(logfile == nullptr){
            ShowMsg("open logfile failed\n");
            return;
        }
        Srv = new Server(logfile,root,ip,port);
        qRegisterMetaType<string>("string");
        connect(Srv,SIGNAL(Msg(const string)),this,SLOT(ShowMsg(const string)));
        Srv->start();
        ui->startButton->setText("Stop");
        ui->browseButton->setVisible(false);
    }

    else{
        Srv->stop();
        ui->startButton->setText("Start");
        ui->browseButton->setVisible(true);
        delete Srv;
        fclose(logfile);
    }
}


void QStr2str(const QString& Qstr,string& str){
    str = string((const char*)(Qstr.toLocal8Bit()));
}
