#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QThread>

#include "network.h"

class QListWidget;
class QLabel;

// The window (runs in the MAIN thread). It shows the list and the three buttons,
// opens the Add/Update form, and updates the list when a response arrives.
// All network work is done by the Network object in its own thread.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Delete button (connected in the constructor). Add and Update buttons call
    // openForm() directly through small lambdas, so they need no slot here.
    void onDeleteClicked();

    // Responses from the server, emitted by Network in the network thread.
    // Because the threads differ, Qt delivers them safely through the main
    // thread's event loop, so it is safe to touch the widgets here.
    void onAddResponse(Protocol::AddDataResp response);
    void onUpdateResponse(Protocol::UpdateDataResp response);
    void onDeleteResponse(Protocol::DeleteDataResp response);
    void onStatusMessage(const QString &message);

private:
    // One row of data kept by the client (the list shows a copy as text).
    struct Record {
        unsigned int uniqueId;
        float lat;
        float longi;
        QString comment;
    };

    void openForm(bool updateMode);                         // Add/Update dialog
    void storeRecord(const Protocol::AddDataResp &response); // add new or replace existing
    void refreshList();                                     // redraw list from m_records
    int selectedRow() const;                                // selected row, -1 if none

    QListWidget *m_list = nullptr;
    QLabel *m_statusLabel = nullptr;

    QThread m_networkThread;      // the thread that runs m_network
    Network *m_network = nullptr; // lives in m_networkThread, not in this thread

    QVector<Record> m_records;    // the client's data, same order as the list rows
};

#endif