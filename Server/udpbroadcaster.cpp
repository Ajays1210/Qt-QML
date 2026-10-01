#include "udpbroadcaster.h"

UdpBroadcaster::UdpBroadcaster(QObject *parent)
    : QObject(parent)
{
}

// Runs inside the UDP thread, so the socket belongs to this thread.
void UdpBroadcaster::start()
{
    m_socket = new QUdpSocket(this);
    emit logMessage("UDP sender ready, target port 4002");
}

void UdpBroadcaster::stop()
{
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

// Receives the response bytes from the TCP thread (queued signal) and
// sends them as one UDP datagram to port 4002.
// We use 127.0.0.1 (loopback) instead of 255.255.255.255 because loopback
// works even with no LAN/network connected, so the client on this same PC
// receives it.
void UdpBroadcaster::broadcast(QByteArray data)
{
    if (!m_socket)
        return;

    // Try a real broadcast first (what the assignment asks for).
    qint64 sent = m_socket->writeDatagram(data, QHostAddress::Broadcast, 4002);

    // With no network interface (e.g. office PC with no LAN) the send fails.
    // Fall back to loopback so a client on the same PC still receives it.
    if (sent < 0) {
        emit logMessage("Broadcast failed (" + m_socket->errorString() +
                        "), falling back to localhost");
        sent = m_socket->writeDatagram(data, QHostAddress::LocalHost, 4002);
    }

    if (sent < 0)
        emit logMessage("UDP send failed: " + m_socket->errorString());
    else
        emit logMessage(QString("UDP sent %1 bytes to port 4002").arg(sent));
}