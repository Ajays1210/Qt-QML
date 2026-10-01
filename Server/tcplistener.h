#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QMap>

#include "protocol.h"

// Runs in the TCP thread. Accepts clients on port 4001, reads the command
// structs, updates the data store, builds a response with Ack, and hands it
// to the UDP thread through the response() signal.
class TcpListener : public QObject
{
    Q_OBJECT

public:
    explicit TcpListener(QObject *parent = nullptr);

public slots:
    void start();   // called when TCP thread starts (QThread::started) -> opens port 4001
    void stop();    // called from Server::stop() -> closes everything

signals:
    // Emitted after every command. Connected in Server to UdpBroadcaster::broadcast.
    // Carries the raw bytes of AddDataResp / DeleteDataResp.
    void response(QByteArray data);
    // Emitted for status text. Connected in Server, printed in main thread.
    void logMessage(const QString &message);

private slots:
    void onNewConnection();   // QTcpServer::newConnection -> a client connected

private:
    void processBuffer(QTcpSocket *socket);   // splits received bytes into full commands

    QTcpServer *m_server = nullptr;
    // Per-client receive buffer. TCP is a byte stream, so one read may contain
    // half a struct or two structs. We collect bytes here until one is complete.
    QHash<QTcpSocket*, QByteArray> m_buffers;
    // The "database": UniqueID -> stored data. Update and Add share the same struct.
    QMap<unsigned int, Protocol::AddDataCmd> m_records;
};

#endif