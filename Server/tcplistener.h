#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QHash>

#include "protocol.h"

class TcpListener : public QObject
{
    Q_OBJECT

public:
    explicit TcpListener(QObject *parent = nullptr);

public slots:
    void start();
    void stop();

signals:
    void addResponse(Protocol::AddDataResp response);
    void updateResponse(Protocol::UpdateDataResp response);
    void deleteResponse(Protocol::DeleteDataResp response);
    void logMessage(const QString &message);

private slots:
    void onNewConnection();

private:
    struct Record {
        unsigned int uniqueId;
        float lat;
        float longi;
        QString comment;
    };

    void processSocket(QTcpSocket *socket);
    void processBuffer(QTcpSocket *socket);
    bool addRecord(const Protocol::AddDataCmd &cmd);
    bool updateRecord(const Protocol::UpdateDataCmd &cmd);
    bool deleteRecord(unsigned int uniqueId);
    Record *findRecord(unsigned int uniqueId);

    QTcpServer *m_server = nullptr;
    QVector<QTcpSocket*> m_clients;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    QVector<Record> m_records;
};

#endif