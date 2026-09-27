#include "network.h"

#include <QHostAddress>
#include <cstring>

namespace {
constexpr char SERVER_IP[] = "127.0.0.1";
constexpr unsigned short TCP_PORT = 4001;
constexpr unsigned short UDP_PORT = 4002;
}

Network::Network(QObject *parent)
    : QObject(parent)
{
}

void Network::start()
{
    m_tcp = new QTcpSocket(this);
    m_udp = new QUdpSocket(this);

    connect(m_tcp, &QTcpSocket::connected,
            this, &Network::onTcpConnected);

    connect(m_tcp, &QTcpSocket::errorOccurred,
            this, &Network::onTcpError);

    connect(m_udp, &QUdpSocket::readyRead,
            this, &Network::onUdpReadyRead);

    const bool bound = m_udp->bind(QHostAddress::AnyIPv4,
                                   UDP_PORT,
                                   QUdpSocket::ShareAddress |
                                       QUdpSocket::ReuseAddressHint);

    if (bound)
        emit statusMessage("UDP listener bound to port 4002");
    else
        emit statusMessage("UDP bind failed: " + m_udp->errorString());

    m_tcp->connectToHost(SERVER_IP, TCP_PORT);
}

void Network::stop()
{
    if (m_tcp) {
        m_tcp->disconnectFromHost();
        m_tcp->deleteLater();
        m_tcp = nullptr;
    }

    if (m_udp) {
        m_udp->close();
        m_udp->deleteLater();
        m_udp = nullptr;
    }
}

bool Network::ensureTcpConnection()
{
    if (!m_tcp)
        return false;

    if (m_tcp->state() == QAbstractSocket::ConnectedState)
        return true;

    m_tcp->connectToHost(SERVER_IP, TCP_PORT);

    if (!m_tcp->waitForConnected(1000)) {
        emit statusMessage("TCP connection failed: " + m_tcp->errorString());
        return false;
    }

    return true;
}

void Network::onTcpConnected()
{
    emit statusMessage("Connected to server TCP 4001");
}

void Network::onTcpError(QAbstractSocket::SocketError)
{
    emit statusMessage("TCP error: " + m_tcp->errorString());
}

void Network::sendAdd(unsigned int uniqueId,
                      float lat,
                      float longi,
                      const QString &comment)
{
    if (!ensureTcpConnection())
        return;

    Protocol::AddDataCmd cmd{};
    cmd.cmdID = Protocol::CMD_ADD;
    cmd.UniqueID = uniqueId;
    cmd.lat = lat;
    cmd.longi = longi;

    QByteArray text = comment.toUtf8();
    text.truncate(49);
    std::memcpy(cmd.comment, text.constData(), text.size());
    cmd.comment[text.size()] = '\0';

    m_tcp->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
    m_tcp->flush();
}

void Network::sendUpdate(unsigned int uniqueId,
                         float lat,
                         float longi,
                         const QString &comment)
{
    if (!ensureTcpConnection())
        return;

    Protocol::UpdateDataCmd cmd{};
    cmd.cmdID = Protocol::CMD_UPDATE;
    cmd.UniqueID = uniqueId;
    cmd.lat = lat;
    cmd.longi = longi;

    QByteArray text = comment.toUtf8();
    text.truncate(49);
    std::memcpy(cmd.comment, text.constData(), text.size());
    cmd.comment[text.size()] = '\0';

    m_tcp->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
    m_tcp->flush();
}

void Network::sendDelete(unsigned int uniqueId)
{
    if (!ensureTcpConnection())
        return;

    Protocol::DeleteDataCmd cmd{};
    cmd.cmdID = Protocol::CMD_DELETE;
    cmd.UniqueID = uniqueId;

    m_tcp->write(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
    m_tcp->flush();
}

void Network::onUdpReadyRead()
{
    while (m_udp && m_udp->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_udp->pendingDatagramSize()));

        QHostAddress sender;
        quint16 senderPort = 0;

        m_udp->readDatagram(datagram.data(), datagram.size(),
                            &sender, &senderPort);

        if (datagram.size() == static_cast<int>(sizeof(Protocol::AddDataResp))) {
            Protocol::AddDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));

            if (response.respID == Protocol::RESP_ADD) {
                emit addResponse(response);
                continue;
            }
        }

        if (datagram.size() == static_cast<int>(sizeof(Protocol::UpdateDataResp))) {
            Protocol::UpdateDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));

            if (response.respID == Protocol::RESP_UPDATE) {
                emit updateResponse(response);
                continue;
            }
        }

        if (datagram.size() == static_cast<int>(sizeof(Protocol::DeleteDataResp))) {
            Protocol::DeleteDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));

            if (response.respID == Protocol::RESP_DELETE) {
                emit deleteResponse(response);
                continue;
            }
        }

        emit statusMessage("Received unknown UDP packet");
    }
}