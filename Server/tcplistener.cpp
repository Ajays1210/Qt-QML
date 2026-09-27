#include "tcplistener.h"

#include <cstring>

TcpListener::TcpListener(QObject *parent)
    : QObject(parent)
{
}

void TcpListener::start()
{
    m_server = new QTcpServer(this);

    connect(m_server, &QTcpServer::newConnection,
            this, &TcpListener::onNewConnection);

    if (!m_server->listen(QHostAddress::AnyIPv4, 4001)) {
        emit logMessage("Could not listen on TCP port 4001: " + m_server->errorString());
        return;
    }

    emit logMessage("Server listening on TCP port 4001");
}

void TcpListener::stop()
{
    for (QTcpSocket *socket : m_clients) {
        if (socket) {
            socket->disconnectFromHost();
            socket->deleteLater();
        }
    }

    m_clients.clear();
    m_buffers.clear();

    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
}

void TcpListener::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();

        m_clients.append(socket);
        m_buffers.insert(socket, QByteArray());

        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            processSocket(socket);
        });

        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            m_clients.removeAll(socket);
            m_buffers.remove(socket);
            socket->deleteLater();
        });

        emit logMessage("Client connected: " + socket->peerAddress().toString());
    }
}

void TcpListener::processSocket(QTcpSocket *socket)
{
    if (!socket)
        return;

    m_buffers[socket].append(socket->readAll());
    processBuffer(socket);
}

void TcpListener::processBuffer(QTcpSocket *socket)
{
    QByteArray &buffer = m_buffers[socket];

    while (!buffer.isEmpty()) {
        unsigned char cmdId = static_cast<unsigned char>(buffer.at(0));

        int requiredSize = 0;

        if (cmdId == Protocol::CMD_ADD)
            requiredSize = sizeof(Protocol::AddDataCmd);
        else if (cmdId == Protocol::CMD_UPDATE)
            requiredSize = sizeof(Protocol::UpdateDataCmd);
        else if (cmdId == Protocol::CMD_DELETE)
            requiredSize = sizeof(Protocol::DeleteDataCmd);
        else {
            emit logMessage("Unknown command ID");
            buffer.clear();
            return;
        }

        if (buffer.size() < requiredSize)
            return;

        QByteArray packet = buffer.left(requiredSize);
        buffer.remove(0, requiredSize);

        if (cmdId == Protocol::CMD_ADD) {
            Protocol::AddDataCmd cmd{};
            std::memcpy(&cmd, packet.constData(), sizeof(cmd));

            Protocol::AddDataResp response{};
            response.respID = Protocol::RESP_ADD;
            response.UniqueID = cmd.UniqueID;
            response.lat = cmd.lat;
            response.longi = cmd.longi;
            std::memcpy(response.comment, cmd.comment, sizeof(response.comment));

            if (addRecord(cmd)) {
                response.Ack = Protocol::ACK_SUCCESS;
                emit logMessage(QString("ADD success: ID=%1").arg(cmd.UniqueID));
            } else {
                response.Ack = Protocol::ACK_FAIL;
                emit logMessage(QString("ADD failed: ID=%1 already exists").arg(cmd.UniqueID));
            }

            emit addResponse(response);
        }
        else if (cmdId == Protocol::CMD_UPDATE) {
            Protocol::UpdateDataCmd cmd{};
            std::memcpy(&cmd, packet.constData(), sizeof(cmd));

            Protocol::UpdateDataResp response{};
            response.respID = Protocol::RESP_UPDATE;
            response.UniqueID = cmd.UniqueID;
            response.lat = cmd.lat;
            response.longi = cmd.longi;
            std::memcpy(response.comment, cmd.comment, sizeof(response.comment));

            if (updateRecord(cmd)) {
                response.Ack = Protocol::ACK_SUCCESS;
                emit logMessage(QString("UPDATE success: ID=%1").arg(cmd.UniqueID));
            } else {
                response.Ack = Protocol::ACK_FAIL;
                emit logMessage(QString("UPDATE failed: ID=%1 not found").arg(cmd.UniqueID));
            }

            emit updateResponse(response);
        }
        else if (cmdId == Protocol::CMD_DELETE) {
            Protocol::DeleteDataCmd cmd{};
            std::memcpy(&cmd, packet.constData(), sizeof(cmd));

            Protocol::DeleteDataResp response{};
            response.respID = Protocol::RESP_DELETE;
            response.UniqueID = cmd.UniqueID;

            if (deleteRecord(cmd.UniqueID)) {
                response.Ack = Protocol::ACK_SUCCESS;
                emit logMessage(QString("DELETE success: ID=%1").arg(cmd.UniqueID));
            } else {
                response.Ack = Protocol::ACK_FAIL;
                emit logMessage(QString("DELETE failed: ID=%1 not found").arg(cmd.UniqueID));
            }

            emit deleteResponse(response);
        }
    }
}

TcpListener::Record *TcpListener::findRecord(unsigned int uniqueId)
{
    for (Record &record : m_records) {
        if (record.uniqueId == uniqueId)
            return &record;
    }

    return nullptr;
}

bool TcpListener::addRecord(const Protocol::AddDataCmd &cmd)
{
    if (findRecord(cmd.UniqueID))
        return false;

    Record record;
    record.uniqueId = cmd.UniqueID;
    record.lat = cmd.lat;
    record.longi = cmd.longi;
    record.comment = QString::fromUtf8(cmd.comment);

    m_records.append(record);
    return true;
}

bool TcpListener::updateRecord(const Protocol::UpdateDataCmd &cmd)
{
    Record *record = findRecord(cmd.UniqueID);

    if (!record)
        return false;

    record->lat = cmd.lat;
    record->longi = cmd.longi;
    record->comment = QString::fromUtf8(cmd.comment);

    return true;
}

bool TcpListener::deleteRecord(unsigned int uniqueId)
{
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].uniqueId == uniqueId) {
            m_records.removeAt(i);
            return true;
        }
    }

    return false;
}