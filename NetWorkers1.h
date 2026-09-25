// NetWorkers.h
// TcpSender runs on the "sending" thread: it connects to the server and
// writes commands (TCP 4001).
// UdpReceiver runs on the "receiving" thread: it listens for the
// server's broadcasts (UDP 4002).
#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>

#include "Protocol.h"

class TcpSender : public QObject {
    Q_OBJECT
public:
    TcpSender(QString serverHost, unsigned short serverPort, QObject *parent = nullptr)
        : QObject(parent), host(serverHost), port(serverPort) {}

public slots:
    void start() {
        socket = new QTcpSocket(this);
    }

    void sendCommand(QByteArray frame) {
        if (socket == nullptr)
            return;

        if (socket->state() != QAbstractSocket::ConnectedState) {
            socket->connectToHost(host, port);
            bool connected = socket->waitForConnected(2000);
            if (!connected) {
                qWarning() << "Could not connect to server";
                return;
            }
        }

        socket->write(frame);
        socket->waitForBytesWritten(2000);
    }

private:
    QString host;
    unsigned short port;
    QTcpSocket *socket = nullptr;
};

class UdpReceiver : public QObject {
    Q_OBJECT
public:
    UdpReceiver(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void recordResponse(int respId, bool ok, unsigned int id, float lat, float lon, QString comment);
    void deleteResponse(bool ok, unsigned int id);

public slots:
    void start() {
        socket = new QUdpSocket(this);
        socket->bind(QHostAddress::AnyIPv4, UDP_PORT, QUdpSocket::ShareAddress);
        connect(socket, &QUdpSocket::readyRead, this, &UdpReceiver::onReadyRead);
    }

private slots:
    void onReadyRead() {
        while (socket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize((int)socket->pendingDatagramSize());
            socket->readDatagram(datagram.data(), datagram.size());
            handleDatagram(datagram);
        }
    }

private:
    void handleDatagram(const QByteArray &d) {
        if (d.isEmpty())
            return;

        unsigned char respId = (unsigned char)d.at(0);

        if (respId == RESP_ADD || respId == RESP_UPDATE) {
            AddDataResp r;
            if (!fromBytes(d, r))
                return;
            r.comment[COMMENT_LENGTH - 1] = 0;
            emit recordResponse(r.respID, r.ack == ACK_SUCCESS, r.uniqueID, r.lat, r.lon, commentToString(r.comment));
        }
        else if (respId == RESP_DELETE) {
            DeleteDataResp r;
            if (!fromBytes(d, r))
                return;
            emit deleteResponse(r.ack == ACK_SUCCESS, r.uniqueID);
        }
    }

    QUdpSocket *socket = nullptr;
};
