#include "tcplistener.h"
#include <cstring>   // memcpy

TcpListener::TcpListener(QObject *parent)
    : QObject(parent)
{
}

// Runs inside the TCP thread. The server is created here (not in the
// constructor) so that it belongs to this thread.
void TcpListener::start()
{
    m_server = new QTcpServer(this);

    // Fires whenever a client connects.
    connect(m_server, &QTcpServer::newConnection,
            this, &TcpListener::onNewConnection);

    if (!m_server->listen(QHostAddress::AnyIPv4, 4001)) {
        emit logMessage("Could not listen on TCP 4001: " + m_server->errorString());
        return;
    }
    emit logMessage("Listening on TCP port 4001");
}

// Closes all client sockets and the server.
void TcpListener::stop()
{
    // Take the sockets out of the hash FIRST, then disconnect. This way the
    // 'disconnected' handler can't modify the container while we loop.
    const QList<QTcpSocket*> sockets = m_buffers.keys();
    m_buffers.clear();

    for (QTcpSocket *s : sockets) {
        s->disconnectFromHost();
        s->deleteLater();
    }

    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
}

// Called by QTcpServer::newConnection.
void TcpListener::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        m_buffers.insert(socket, QByteArray());   // empty buffer for this client

        // New bytes arrived: append to this client's buffer, then try to parse.
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            m_buffers[socket].append(socket->readAll());
            processBuffer(socket);
        });

        // Client left: forget its buffer and free the socket.
        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            m_buffers.remove(socket);
            socket->deleteLater();
        });

        emit logMessage("Client connected: " + socket->peerAddress().toString());
    }
}

// Pulls complete command structs out of the client's buffer, one by one.
// The first byte of every command is cmdID, which tells us the struct size.
void TcpListener::processBuffer(QTcpSocket *socket)
{
    QByteArray &buffer = m_buffers[socket];

    // Small helper: turn a response struct into bytes and emit it.
    // This goes to the UDP thread -> UdpBroadcaster::broadcast().
    auto send = [this](const auto &resp) {
        emit response(QByteArray(reinterpret_cast<const char*>(&resp), sizeof(resp)));
    };

    while (!buffer.isEmpty()) {
        const unsigned char cmdId = static_cast<unsigned char>(buffer.at(0));

        // 1) Find out how many bytes this command needs.
        int size = 0;
        switch (cmdId) {
        case Protocol::CMD_ADD:    size = sizeof(Protocol::AddDataCmd);    break;
        case Protocol::CMD_UPDATE: size = sizeof(Protocol::UpdateDataCmd); break;
        case Protocol::CMD_DELETE: size = sizeof(Protocol::DeleteDataCmd); break;
        default:
            emit logMessage("Unknown command ID, buffer cleared");
            buffer.clear();
            return;
        }

        // 2) Not all bytes arrived yet -> wait for the next readyRead.
        if (buffer.size() < size)
            return;

        // 3) Cut exactly one command out of the buffer.
        const QByteArray packet = buffer.left(size);
        buffer.remove(0, size);

        // 4) Handle it.
        if (cmdId == Protocol::CMD_ADD || cmdId == Protocol::CMD_UPDATE) {
            Protocol::AddDataCmd cmd{};
            std::memcpy(&cmd, packet.constData(), sizeof(cmd));

            // Response echoes the same data, plus Ack.
            Protocol::AddDataResp resp{};
            resp.respID = (cmdId == Protocol::CMD_ADD) ? Protocol::RESP_ADD
                                                       : Protocol::RESP_UPDATE;
            resp.UniqueID = cmd.UniqueID;
            resp.lat = cmd.lat;
            resp.longi = cmd.longi;
            std::memcpy(resp.comment, cmd.comment, sizeof(resp.comment));

            const bool exists = m_records.contains(cmd.UniqueID);
            // ADD fails if ID already exists; UPDATE fails if it does NOT exist.
            const bool ok = (cmdId == Protocol::CMD_ADD) ? !exists : exists;

            if (ok) {
                m_records[cmd.UniqueID] = cmd;   // insert new, or overwrite existing
                resp.Ack = Protocol::ACK_SUCCESS;
            } else {
                resp.Ack = Protocol::ACK_FAIL;
            }

            emit logMessage(QString("%1 ID=%2 -> %3")
                                .arg(cmdId == Protocol::CMD_ADD ? "ADD" : "UPDATE")
                                .arg(cmd.UniqueID)
                                .arg(ok ? "success" : "fail"));
            send(resp);
        }
        else {   // CMD_DELETE
            Protocol::DeleteDataCmd cmd{};
            std::memcpy(&cmd, packet.constData(), sizeof(cmd));

            Protocol::DeleteDataResp resp{};
            resp.respID = Protocol::RESP_DELETE;
            resp.UniqueID = cmd.UniqueID;

            // remove() returns how many items it removed (0 = ID not found).
            const bool ok = m_records.remove(cmd.UniqueID) > 0;
            resp.Ack = ok ? Protocol::ACK_SUCCESS : Protocol::ACK_FAIL;

            emit logMessage(QString("DELETE ID=%1 -> %2")
                                .arg(cmd.UniqueID)
                                .arg(ok ? "success" : "fail"));
            send(resp);
        }
    }
}