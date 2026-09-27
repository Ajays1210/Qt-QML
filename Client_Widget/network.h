#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>

#include "protocol.h"

class Network : public QObject
{
    Q_OBJECT

public:
    explicit Network(QObject *parent = nullptr);

public slots:
    void start();
    void stop();

    void sendAdd(unsigned int uniqueId, float lat, float longi, const QString &comment);
    void sendUpdate(unsigned int uniqueId, float lat, float longi, const QString &comment);
    void sendDelete(unsigned int uniqueId);

signals:
    void addResponse(Protocol::AddDataResp response);
    void updateResponse(Protocol::UpdateDataResp response);
    void deleteResponse(Protocol::DeleteDataResp response);
    void statusMessage(const QString &message);

private slots:
    void onTcpConnected();
    void onTcpError(QAbstractSocket::SocketError error);
    void onUdpReadyRead();

private:
    bool ensureTcpConnection();

    QTcpSocket *m_tcp = nullptr;
    QUdpSocket *m_udp = nullptr;
};

#endif