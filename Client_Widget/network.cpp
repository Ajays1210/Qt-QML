#include "network.h"

#include <QHostAddress>
#include <cstring>

namespace {
constexpr char SERVER_IP[] = "127.0.0.1";   // server runs on this same PC
constexpr unsigned short TCP_PORT = 4001;   // we SEND commands here
constexpr unsigned short UDP_PORT = 4002;   // we RECEIVE responses here
}

Network::Network(QObject *parent)
    : QObject(parent)
{
}

// Runs inside the network thread. The sockets are created here (not in the
// constructor) so that they belong to this thread.
void Network::start()
{
    m_tcp = new QTcpSocket(this);
    m_udp = new QUdpSocket(this);

    connect(m_tcp, &QTcpSocket::connected,
            this, &Network::onTcpConnected);

    connect(m_tcp, &QTcpSocket::errorOccurred,
            this, &Network::onTcpError);

    // Whenever a UDP datagram arrives, onUdpReadyRead() runs.
    connect(m_udp, &QUdpSocket::readyRead,
            this, &Network::onUdpReadyRead);

    // Listen on port 4002 on every IPv4 address. ShareAddress lets several
    // clients use the same port at the same time.
    const bool bound = m_udp->bind(QHostAddress::AnyIPv4,
                                   UDP_PORT,
                                   QUdpSocket::ShareAddress |
                                       QUdpSocket::ReuseAddressHint);

    if (bound)
        emit statusMessage("UDP listener bound to port 4002");
    else
        emit statusMessage("UDP bind failed: " + m_udp->errorString());

    // Start connecting to the server now. This returns immediately and
    // onTcpConnected() runs when the connection is ready.
    m_tcp->connectToHost(SERVER_IP, TCP_PORT);
}

// Returns true when TCP is connected and ready to send. If the connection was
// lost (or the server was not running at startup), it tries to reconnect.
bool Network::ensureTcpConnection()
{
    if (!m_tcp)
        return false;

    if (m_tcp->state() == QAbstractSocket::ConnectedState)
        return true;

    // Start a new connection only if none is already in progress
    // (calling connectToHost while connecting makes Qt print a warning).
    if (m_tcp->state() == QAbstractSocket::UnconnectedState)
        m_tcp->connectToHost(SERVER_IP, TCP_PORT);

    // Wait up to 1 second. This blocks only the network thread, never the GUI.
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

// Writes the bytes to the server (after making sure we are connected).
void Network::sendBytes(const char *data, int size)
{
    if (!ensureTcpConnection())
        return;

    m_tcp->write(data, size);
    m_tcp->flush();   // push the bytes out now instead of waiting
}

// Add and Update use the same struct (63 bytes). Only cmdID differs.
void Network::sendRecord(unsigned char cmdId, unsigned int uniqueId,
                         float lat, float longi, const QString &comment)
{
    Protocol::AddDataCmd cmd{};   // {} = every byte starts at zero
    cmd.cmdID = cmdId;
    cmd.UniqueID = uniqueId;
    cmd.lat = lat;
    cmd.longi = longi;

    // Copy at most 49 bytes. The array is char[50] and was zero-filled above,
    // so the last byte is always '\0' (end of text).
    QByteArray text = comment.toUtf8();
    text.truncate(49);
    std::memcpy(cmd.comment, text.constData(), text.size());

    sendBytes(reinterpret_cast<const char*>(&cmd), static_cast<int>(sizeof(cmd)));
}

void Network::sendAdd(unsigned int uniqueId, float lat, float longi, const QString &comment)
{
    sendRecord(Protocol::CMD_ADD, uniqueId, lat, longi, comment);
}

void Network::sendUpdate(unsigned int uniqueId, float lat, float longi, const QString &comment)
{
    sendRecord(Protocol::CMD_UPDATE, uniqueId, lat, longi, comment);
}

// Delete only needs the ID (5 bytes).
void Network::sendDelete(unsigned int uniqueId)
{
    Protocol::DeleteDataCmd cmd{};
    cmd.cmdID = Protocol::CMD_DELETE;
    cmd.UniqueID = uniqueId;

    sendBytes(reinterpret_cast<const char*>(&cmd), static_cast<int>(sizeof(cmd)));
}

// Reads every waiting datagram. The first byte (respID) tells which response
// it is. We also check the size, so a wrong or damaged packet is ignored.
void Network::onUdpReadyRead()
{
    while (m_udp && m_udp->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_udp->pendingDatagramSize()));
        m_udp->readDatagram(datagram.data(), datagram.size());

        if (datagram.isEmpty())
            continue;

        const unsigned char respId = static_cast<unsigned char>(datagram.at(0));
        const int size = datagram.size();

        if (respId == Protocol::RESP_ADD &&
            size == static_cast<int>(sizeof(Protocol::AddDataResp))) {
            Protocol::AddDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));
            emit addResponse(response);     // -> MainWindow::onAddResponse
        }
        else if (respId == Protocol::RESP_UPDATE &&
                 size == static_cast<int>(sizeof(Protocol::UpdateDataResp))) {
            Protocol::UpdateDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));
            emit updateResponse(response);  // -> MainWindow::onUpdateResponse
        }
        else if (respId == Protocol::RESP_DELETE &&
                 size == static_cast<int>(sizeof(Protocol::DeleteDataResp))) {
            Protocol::DeleteDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));
            emit deleteResponse(response);  // -> MainWindow::onDeleteResponse
        }
        else {
            emit statusMessage("Received unknown UDP packet");
        }
    }
}