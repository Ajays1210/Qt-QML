#include "mainwindow.h"
#include "ui_mainwindow.h"

// #include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // connect(ui->helloButton,
    //         &QPushButton::clicked,
    //         this,
    //         &MainWindow::sayHello);

    connect(ui->applyButton,
            &QPushButton::clicked,
            this,
            &MainWindow::applyData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// void MainWindow::sayHello()
// {
//     QString name = ui->nameEdit->text();
//     ui->messageLabel->setText("Hello " + name);
// }

void MainWindow::applyData()
{
    // QString title = ui->titleLabel->text();
    QString id = ui->idEdit->text();
    QString lat = ui->latEdit->text();
    QString lon = ui->longEdit->text();
    QString comment = ui->commentEdit->text();
    // QString status = ui->statusLabel->text();

    ui->statusEditLabel->setText(
        "ID: " + id +
        " | Lat: " + lat +
        " | Long: " + lon +
        " | Comment: " + comment
        );
}