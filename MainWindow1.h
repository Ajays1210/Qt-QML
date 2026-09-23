// MainWindow.h
// The main window: a list plus Add / Update / Delete buttons.
// This class runs on the main (GUI) thread. All network work happens in
// TcpSender and UdpReceiver, which run on their own threads.
#pragma once

#include <QMainWindow>
#include <QVector>
#include <QThread>

class QListWidget;
class QPushButton;
class TcpSender;
class UdpReceiver;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QString serverHost, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void commandReady(QByteArray frame);   // sent to TcpSender

private slots:
    void onAddClicked();
    void onUpdateClicked();
    void onDeleteClicked();
    void onSelectionChanged();

    void onRecordResponse(int respId, bool ok, unsigned int id, float lat, float lon, QString comment);
    void onDeleteResponse(bool ok, unsigned int id);

private:
    struct Record {
        unsigned int id;
        float lat;
        float lon;
        QString comment;
    };

    int findRecord(unsigned int id);
    void refreshList();
    unsigned int nextFreeId();
    unsigned int selectedId();
    void sendAddOrUpdate(unsigned char cmdId, unsigned int id, float lat, float lon, QString comment);

    QListWidget *list;
    QPushButton *addButton;
    QPushButton *updateButton;
    QPushButton *deleteButton;

    QVector<Record> records;

    QThread sendThread;
    QThread receiveThread;
};
