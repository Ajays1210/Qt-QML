// main.cpp (server)
// This creates two threads: one to receive commands (TCP 4001) and one
// to send broadcasts (UDP 4002), and connects them together.
//
// Usage: task_server            (broadcasts to 255.255.255.255)
//        task_server 127.0.0.1  (for testing on one PC)
#include <QCoreApplication>
#include <QThread>
#include <QHostAddress>

#include "ReceiverWorker.h"
#include "SenderWorker.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QHostAddress targetAddress = QHostAddress::Broadcast;
    if (argc > 1)
        targetAddress = QHostAddress(QString(argv[1]));

    QThread receiveThread;
    QThread sendThread;

    ReceiverWorker *receiver = new ReceiverWorker();
    SenderWorker *sender = new SenderWorker(targetAddress);

    // Move each worker to its own thread.
    receiver->moveToThread(&receiveThread);
    sender->moveToThread(&sendThread);

    // When each thread starts, call that worker's start() function.
    QObject::connect(&receiveThread, &QThread::started, receiver, &ReceiverWorker::start);
    QObject::connect(&sendThread, &QThread::started, sender, &SenderWorker::start);

    // When the receiver has a response ready, send it out as a broadcast.
    QObject::connect(receiver, &ReceiverWorker::responseReady, sender, &SenderWorker::sendBroadcast);

    receiveThread.start();
    sendThread.start();

    qInfo() << "Server running. Press Ctrl+C to stop.";
    return app.exec();
}
