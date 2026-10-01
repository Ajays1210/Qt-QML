#include "udpreceiver.h"
#include "protocol.h"

#include <QHostAddress>
#include <QDebug>
#include <cstring>

// Turns the fixed char[50] from the struct into a QString.
// It stops at the first '\0', or at the end of the array if there is none,
// so it never reads outside the 50 bytes.
static QString getComment(const char *data, unsigned int size)
{
    unsigned int length = 0;
    while (length < size && data[length] != '\0')
        ++length;

    return QString::fromUtf8(data, length);
}

UdpReceiver::UdpReceiver(QObject *parent)
    : QObject(parent),
    m_socket(nullptr)
{
}

// Runs inside the receiver thread.
void UdpReceiver::startListening()
{
    m_socket = new QUdpSocket(this);

    // Listen on every IPv4 address, port 4002.
    // ShareAddress + ReuseAddressHint allow other programs (for example a
    // second client) to use the same port at the same time.
    bool ok = m_socket->bind(
        QHostAddress::AnyIPv4,
        4002,
        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (!ok) {
        qWarning() << "Client: UDP bind failed:" << m_socket->errorString();
        return;
    }

    qInfo() << "Client: listening on UDP 4002";

    // Whenever data arrives, run onReadyRead.
    connect(m_socket, &QUdpSocket::readyRead,
            this, &UdpReceiver::onReadyRead);
}

// Reads every waiting datagram, finds the response type from the first byte,
// converts the bytes back into the struct, and emits the matching signal.
void UdpReceiver::onReadyRead()
{
    while (m_socket->hasPendingDatagrams())
    {
        // Make room for exactly one datagram, then read it.
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(datagram.data(), datagram.size());

        if (datagram.isEmpty())
            continue;

        // First byte of every response = respID.
        unsigned char responseId = static_cast<unsigned char>(datagram.at(0));

        // For each type, check BOTH the ID and the size, so a wrong or
        // damaged packet is ignored instead of being misread.
        if (responseId == Protocol::RESP_ADD &&
            datagram.size() == static_cast<int>(sizeof(Protocol::AddDataResp)))
        {
            Protocol::AddDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));
            QString comment = getComment(response.comment, sizeof(response.comment));

            emit addResponse(response.Ack, response.UniqueID, response.lat, response.longi, comment);
        }
        else if (responseId == Protocol::RESP_UPDATE &&
                 datagram.size() == static_cast<int>(sizeof(Protocol::UpdateDataResp)))
        {
            Protocol::UpdateDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));
            QString comment = getComment(response.comment, sizeof(response.comment));

            emit updateResponse(response.Ack, response.UniqueID, response.lat, response.longi, comment);
        }
        else if (responseId == Protocol::RESP_DELETE &&
                 datagram.size() == static_cast<int>(sizeof(Protocol::DeleteDataResp)))
        {
            Protocol::DeleteDataResp response{};
            std::memcpy(&response, datagram.constData(), sizeof(response));

            emit deleteResponse(response.Ack, response.UniqueID);
        }
    }
}