#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QThread>

class TcpListener;
class UdpBroadcaster;

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    void start();
    void stop();

private:
    QThread m_tcpThread;
    QThread m_udpThread;

    TcpListener *m_tcpListener = nullptr;
    UdpBroadcaster *m_udpBroadcaster = nullptr;
};

#endif