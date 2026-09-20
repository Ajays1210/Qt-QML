#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->applyButton,
            &QPushButton::clicked,
            this,
            &MainWindow::applyData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::applyData()
{
    MyData data;

    data.id = ui->idEdit->text().toUInt();
    data.latitude = ui->latEdit->text().toFloat();
    data.longitude = ui->longEdit->text().toFloat();
    data.comment = ui->commentEdit->text();

    data.isValid = true;

    ui->validEditLabel->setText("Yes");

    ui->statusEditLabel->setText(
        "ID: " + QString::number(data.id) +
        " | Lat: " + QString::number(data.latitude) +
        " | Long: " + QString::number(data.longitude) +
        " | Comment: " + data.comment +
        " | Status: " + QString(data.isValid ? "Yes" : "No")
        );
}