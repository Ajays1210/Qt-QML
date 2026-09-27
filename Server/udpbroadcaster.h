#ifndef UDPBROADCASTER_H
#define UDPBROADCASTER_H

#include <QObject>
#include <QUdpSocket>

#include "protocol.h"

class UdpBroadcaster : public QObject
{
    Q_OBJECT

public:
    explicit UdpBroadcaster(QObject *parent = nullptr);

public slots:
    void start();
    void stop();

    void broadcastAddResponse(Protocol::AddDataResp response);
    void broadcastUpdateResponse(Protocol::UpdateDataResp response);
    void broadcastDeleteResponse(Protocol::DeleteDataResp response);

signals:
    void logMessage(const QString &message);

private:
    QUdpSocket *m_socket = nullptr;
};

#endif