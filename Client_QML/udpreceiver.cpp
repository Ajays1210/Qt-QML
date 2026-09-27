#include "udpreceiver.h"
#include "protocol.h"

#include <QHostAddress>
#include <QDebug>
#include <cstring>

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

void UdpReceiver::startListening()
{
    m_socket = new QUdpSocket(this);

    bool ok = m_socket->bind(
        QHostAddress::AnyIPv4,
        4002,
        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (!ok) {
        qWarning() << "Client: UDP bind failed:" << m_socket->errorString();
        return;
    }

    qInfo() << "Client: listening on UDP 4002";

    connect(m_socket, &QUdpSocket::readyRead,
            this, &UdpReceiver::onReadyRead);
}

void UdpReceiver::onReadyRead()
{
    while (m_socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(datagram.data(), datagram.size());

        if (datagram.isEmpty())
            continue;

        unsigned char responseId = static_cast<unsigned char>(datagram.at(0));

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