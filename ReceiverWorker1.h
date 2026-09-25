// ReceiverWorker.h
// This object runs on its own thread. It listens for a TCP connection on
// port 4001, reads commands sent by the client, updates our list of
// records, and asks the SenderWorker to broadcast the result.
#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QDebug>

#include "Protocol.h"

class ReceiverWorker : public QObject {
    Q_OBJECT
public:
    ReceiverWorker(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void responseReady(QByteArray datagram);   // sent to SenderWorker

public slots:
    // This runs once, right when the thread starts.
    void start() {
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &ReceiverWorker::onNewConnection);

        bool ok = server->listen(QHostAddress::Any, TCP_PORT);
        if (!ok)
            qWarning() << "Could not listen on TCP port" << TCP_PORT;
        else
            qInfo() << "Server listening on TCP port" << TCP_PORT;
    }

private slots:
    void onNewConnection() {
        clientSocket = server->nextPendingConnection();
        qInfo() << "Client connected";

        connect(clientSocket, &QTcpSocket::readyRead, this, &ReceiverWorker::onReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &ReceiverWorker::onDisconnected);
    }

    void onDisconnected() {
        qInfo() << "Client disconnected";
        clientSocket->deleteLater();
        clientSocket = nullptr;
        buffer.clear();
    }

    // TCP delivers a stream of bytes, so one command might arrive in two
    // pieces, or two commands might arrive stuck together. We keep collecting
    // bytes in "buffer" and only handle a command once all its bytes are in.
    void onReadyRead() {
        buffer += clientSocket->readAll();

        while (!buffer.isEmpty()) {
            unsigned char cmdId = (unsigned char)buffer.at(0);
            int size = commandSize(cmdId);

            if (size == 0) {
                // Unknown command byte — we cannot make sense of the buffer any more.
                buffer.clear();
                return;
            }
            if (buffer.size() < size) {
                // Not all bytes have arrived yet — wait for more.
                return;
            }

            QByteArray frame = buffer.left(size);
            buffer.remove(0, size);
            handleCommand(cmdId, frame);
        }
    }

private:
    // One stored record.
    struct Record {
        unsigned int id;
        float lat;
        float lon;
        QString comment;
    };

    // Finds the index of the record with this ID, or -1 if not found.
    int findRecord(unsigned int id) {
        for (int i = 0; i < records.size(); i++) {
            if (records[i].id == id)
                return i;
        }
        return -1;
    }

    void handleCommand(unsigned char cmdId, const QByteArray &frame) {
        if (cmdId == CMD_ADD || cmdId == CMD_UPDATE) {
            AddDataCmd cmd;
            if (!fromBytes(frame, cmd))
                return;
            cmd.comment[COMMENT_LENGTH - 1] = 0;   // make sure it always ends properly

            int index = findRecord(cmd.uniqueID);
            bool ok;
            if (cmdId == CMD_ADD)
                ok = (index == -1);     // Add only succeeds if the ID is new
            else
                ok = (index != -1);     // Update only succeeds if the ID exists

            if (ok) {
                Record r;
                r.id = cmd.uniqueID;
                r.lat = cmd.lat;
                r.lon = cmd.lon;
                r.comment = commentToString(cmd.comment);

                if (cmdId == CMD_ADD)
                    records.push_back(r);
                else
                    records[index] = r;
            }

            AddDataResp resp;
            resp.respID = (cmdId == CMD_ADD) ? RESP_ADD : RESP_UPDATE;
            resp.ack = ok ? ACK_SUCCESS : ACK_FAIL;
            resp.uniqueID = cmd.uniqueID;
            resp.lat = cmd.lat;
            resp.lon = cmd.lon;
            memcpy(resp.comment, cmd.comment, COMMENT_LENGTH);

            qInfo() << (cmdId == CMD_ADD ? "Add" : "Update") << "id" << cmd.uniqueID << "success:" << ok;
            emit responseReady(toBytes(resp));
        }
        else if (cmdId == CMD_DELETE) {
            DeleteDataCmd cmd;
            if (!fromBytes(frame, cmd))
                return;

            int index = findRecord(cmd.uniqueID);
            bool ok = (index != -1);
            if (ok)
                records.removeAt(index);

            DeleteDataResp resp;
            resp.respID = RESP_DELETE;
            resp.ack = ok ? ACK_SUCCESS : ACK_FAIL;
            resp.uniqueID = cmd.uniqueID;

            qInfo() << "Delete id" << cmd.uniqueID << "success:" << ok;
            emit responseReady(toBytes(resp));
        }
    }

    QTcpServer *server = nullptr;
    QTcpSocket *clientSocket = nullptr;
    QByteArray buffer;
    QVector<Record> records;
};
