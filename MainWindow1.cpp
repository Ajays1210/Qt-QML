#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QStatusBar>
#include <QMessageBox>

#include "NetWorkers.h"
#include "RecordDialog.h"
#include "Protocol.h"

MainWindow::MainWindow(QString serverHost, QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Data Client");
    resize(600, 400);

    list = new QListWidget();
    addButton = new QPushButton("Add");
    updateButton = new QPushButton("Update");
    deleteButton = new QPushButton("Delete");

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->addWidget(addButton);
    buttonRow->addWidget(updateButton);
    buttonRow->addWidget(deleteButton);

    QWidget *central = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(list);
    mainLayout->addLayout(buttonRow);
    setCentralWidget(central);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddClicked);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::onUpdateClicked);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteClicked);
    connect(list, &QListWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);
    onSelectionChanged();   // buttons start disabled since nothing is selected yet

    // Set up the two network worker threads.
    TcpSender *sender = new TcpSender(serverHost, TCP_PORT);
    UdpReceiver *receiver = new UdpReceiver();
    sender->moveToThread(&sendThread);
    receiver->moveToThread(&receiveThread);

    connect(&sendThread, &QThread::started, sender, &TcpSender::start);
    connect(&receiveThread, &QThread::started, receiver, &UdpReceiver::start);

    connect(this, &MainWindow::commandReady, sender, &TcpSender::sendCommand);
    connect(receiver, &UdpReceiver::recordResponse, this, &MainWindow::onRecordResponse);
    connect(receiver, &UdpReceiver::deleteResponse, this, &MainWindow::onDeleteResponse);

    sendThread.start();
    receiveThread.start();

    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {
    sendThread.quit();
    receiveThread.quit();
    sendThread.wait();
    receiveThread.wait();
}

void MainWindow::onAddClicked() {
    RecordDialog dialog("Add", false, nextFreeId(), 0.0, 0.0, "", this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    sendAddOrUpdate(CMD_ADD, dialog.getId(), dialog.getLat(), dialog.getLon(), dialog.getComment());
    statusBar()->showMessage("Add sent, waiting for server...");
}

void MainWindow::onUpdateClicked() {
    unsigned int id = selectedId();
    if (id == 0) {
        QMessageBox::information(this, "Update", "Select an item first.");
        return;
    }

    int index = findRecord(id);
    Record r = records[index];

    RecordDialog dialog("Update", true, id, r.lat, r.lon, r.comment, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    sendAddOrUpdate(CMD_UPDATE, id, dialog.getLat(), dialog.getLon(), dialog.getComment());
    statusBar()->showMessage("Update sent, waiting for server...");
}

void MainWindow::onDeleteClicked() {
    unsigned int id = selectedId();
    if (id == 0) {
        QMessageBox::information(this, "Delete", "Select an item first.");
        return;
    }

    DeleteDataCmd cmd;
    cmd.cmdID = CMD_DELETE;
    cmd.uniqueID = id;
    emit commandReady(toBytes(cmd));

    statusBar()->showMessage("Delete sent, waiting for server...");
}

void MainWindow::onSelectionChanged() {
    bool somethingSelected = (selectedId() != 0);
    updateButton->setEnabled(somethingSelected);
    deleteButton->setEnabled(somethingSelected);
}

void MainWindow::sendAddOrUpdate(unsigned char cmdId, unsigned int id, float lat, float lon, QString comment) {
    AddDataCmd cmd;
    cmd.cmdID = cmdId;
    cmd.uniqueID = id;
    cmd.lat = lat;
    cmd.lon = lon;
    setComment(cmd.comment, comment);
    emit commandReady(toBytes(cmd));
}

// This is called only when a broadcast arrives from the server. The list
// is changed here and nowhere else, and only if ack was success.
void MainWindow::onRecordResponse(int respId, bool ok, unsigned int id, float lat, float lon, QString comment) {
    bool isAdd = (respId == RESP_ADD);

    if (!ok) {
        statusBar()->showMessage(isAdd ? "Add failed (ID already exists)" : "Update failed (ID not found)");
        return;
    }

    int index = findRecord(id);
    Record r;
    r.id = id;
    r.lat = lat;
    r.lon = lon;
    r.comment = comment;

    if (index == -1)
        records.push_back(r);
    else
        records[index] = r;

    refreshList();
    statusBar()->showMessage(isAdd ? "Add successful" : "Update successful");
}

void MainWindow::onDeleteResponse(bool ok, unsigned int id) {
    if (!ok) {
        statusBar()->showMessage("Delete failed (ID not found)");
        return;
    }

    int index = findRecord(id);
    if (index != -1)
        records.removeAt(index);

    refreshList();
    statusBar()->showMessage("Delete successful");
}

int MainWindow::findRecord(unsigned int id) {
    for (int i = 0; i < records.size(); i++) {
        if (records[i].id == id)
            return i;
    }
    return -1;
}

void MainWindow::refreshList() {
    list->clear();
    for (int i = 0; i < records.size(); i++) {
        Record r = records[i];
        QString text = "ID: " + QString::number(r.id)
                     + "   Lat: " + QString::number(r.lat, 'f', 5)
                     + "   Long: " + QString::number(r.lon, 'f', 5)
                     + "   " + r.comment;
        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, r.id);
        list->addItem(item);
    }
}

unsigned int MainWindow::nextFreeId() {
    unsigned int maxId = 0;
    for (int i = 0; i < records.size(); i++) {
        if (records[i].id > maxId)
            maxId = records[i].id;
    }
    return maxId + 1;
}

unsigned int MainWindow::selectedId() {
    QListWidgetItem *item = list->currentItem();
    if (item == nullptr)
        return 0;
    return item->data(Qt::UserRole).toUInt();
}
