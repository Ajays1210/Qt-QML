#ifndef TCPSENDER_H
#define TCPSENDER_H

#include <QObject>
#include <QTcpSocket>

class TcpSender : public QObject
{
    Q_OBJECT

public:
    explicit TcpSender(QObject *parent = nullptr);

public slots:
    void connectToServer();
    void sendCommand(QByteArray data);

private:
    QTcpSocket *m_socket;
};

#endif