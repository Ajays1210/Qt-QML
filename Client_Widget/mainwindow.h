#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QThread>

#include "network.h"

class QListWidget;
class QPushButton;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddClicked();
    void onUpdateClicked();
    void onDeleteClicked();

    void onAddResponse(Protocol::AddDataResp response);
    void onUpdateResponse(Protocol::UpdateDataResp response);
    void onDeleteResponse(Protocol::DeleteDataResp response);
    void onStatusMessage(const QString &message);

private:
    struct Record {
        unsigned int uniqueId;
        float lat;
        float longi;
        QString comment;
    };

    void openForm(bool updateMode);
    bool readForm(unsigned int &uniqueId, float &lat, float &longi, QString &comment);
    void clearForm();
    void refreshList();
    int selectedRow() const;

    QListWidget *m_list = nullptr;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_updateButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QLabel *m_statusLabel = nullptr;

    QThread m_networkThread;
    Network *m_network = nullptr;

    QVector<Record> m_records;
};

#endif