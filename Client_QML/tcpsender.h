#ifndef TCPSENDER_H
#define TCPSENDER_H

#include <QObject>
#include <QTcpSocket>

// Runs in the SENDER thread (set up in main.cpp). Its only job is to send
// command bytes to the server on TCP port 4001.
class TcpSender : public QObject
{
    Q_OBJECT

public:
    explicit TcpSender(QObject *parent = nullptr);

public slots:
    // Called when the sender thread starts (QThread::started, in main.cpp).
    // Creates the socket and connects to the server.
    void connectToServer();

    // Called when Controller emits sendBytes (queued, in main.cpp).
    // Writes the bytes to the server.
    void sendCommand(QByteArray data);

private:
    QTcpSocket *m_socket;   // created in connectToServer so it belongs to this thread
};

#endif