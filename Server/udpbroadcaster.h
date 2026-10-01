#ifndef UDPBROADCASTER_H
#define UDPBROADCASTER_H

#include <QObject>
#include <QUdpSocket>

// Runs in the UDP thread. Its only job: send response bytes out on UDP 4002.
class UdpBroadcaster : public QObject
{
    Q_OBJECT

public:
    explicit UdpBroadcaster(QObject *parent = nullptr);

public slots:
    void start();                 // called when UDP thread starts -> creates the socket
    void stop();                  // called from Server::stop() -> closes the socket
    void broadcast(QByteArray data);   // called via TcpListener::response signal

signals:
    void logMessage(const QString &message);   // status text, printed in main thread

private:
    QUdpSocket *m_socket = nullptr;
};

#endif