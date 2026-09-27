#include "udpbroadcaster.h"

UdpBroadcaster::UdpBroadcaster(QObject *parent)
    : QObject(parent)
{
}

void UdpBroadcaster::start()
{
    m_socket = new QUdpSocket(this);
    emit logMessage("UDP broadcaster ready on port 4002");
}

void UdpBroadcaster::stop()
{
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void UdpBroadcaster::broadcastAddResponse(Protocol::AddDataResp response)
{
    if (!m_socket)
        return;

    const QByteArray data(reinterpret_cast<const char*>(&response), sizeof(response));
    m_socket->writeDatagram(data, QHostAddress::Broadcast, 4002);
    emit logMessage("Broadcast ADD response on UDP 4002");
}

void UdpBroadcaster::broadcastUpdateResponse(Protocol::UpdateDataResp response)
{
    if (!m_socket)
        return;

    const QByteArray data(reinterpret_cast<const char*>(&response), sizeof(response));
    m_socket->writeDatagram(data, QHostAddress::Broadcast, 4002);
    emit logMessage("Broadcast UPDATE response on UDP 4002");
}

void UdpBroadcaster::broadcastDeleteResponse(Protocol::DeleteDataResp response)
{
    if (!m_socket)
        return;

    const QByteArray data(reinterpret_cast<const char*>(&response), sizeof(response));
    m_socket->writeDatagram(data, QHostAddress::Broadcast, 4002);
    emit logMessage("Broadcast DELETE response on UDP 4002");
}