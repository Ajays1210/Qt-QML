// SenderWorker.h
// This object runs on its own thread too. Whenever the ReceiverWorker
// finishes a response, it calls sendBroadcast() here. Qt delivers this
// safely across threads because of how the signal/slot connection is
// made in main.cpp.
#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>

#include "Protocol.h"

class SenderWorker : public QObject {
    Q_OBJECT
public:
    SenderWorker(QHostAddress targetAddress, QObject *parent = nullptr)
        : QObject(parent), target(targetAddress) {}

public slots:
    void start() {
        socket = new QUdpSocket(this);
        qInfo() << "Server broadcasting on UDP port" << UDP_PORT;
    }

    void sendBroadcast(QByteArray datagram) {
        if (socket == nullptr)
            return;
        socket->writeDatagram(datagram, target, UDP_PORT);
    }

private:
    QHostAddress target;
    QUdpSocket *socket = nullptr;
};
