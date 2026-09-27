#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QThread>

#include "model.h"
#include "controller.h"
#include "tcpsender.h"
#include "udpreceiver.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    Model model;
    Controller controller(&model);

    // ---------------- TCP SEND THREAD ----------------
    QThread senderThread;
    auto *sender = new TcpSender;
    sender->moveToThread(&senderThread);

    QObject::connect(&senderThread, &QThread::started,
                     sender, &TcpSender::connectToServer);

    QObject::connect(&controller, &Controller::sendBytes,
                     sender, &TcpSender::sendCommand,
                     Qt::QueuedConnection);

    QObject::connect(&senderThread, &QThread::finished,
                     sender, &QObject::deleteLater);

    senderThread.start();

    // ---------------- UDP RECEIVE THREAD ----------------
    QThread receiverThread;
    auto *receiver = new UdpReceiver;
    receiver->moveToThread(&receiverThread);

    QObject::connect(&receiverThread, &QThread::started,
                     receiver, &UdpReceiver::startListening);

    QObject::connect(receiver, &UdpReceiver::addResponse,
                     &controller, &Controller::onAddResponse,
                     Qt::QueuedConnection);

    QObject::connect(receiver, &UdpReceiver::updateResponse,
                     &controller, &Controller::onUpdateResponse,
                     Qt::QueuedConnection);

    QObject::connect(receiver, &UdpReceiver::deleteResponse,
                     &controller, &Controller::onDeleteResponse,
                     Qt::QueuedConnection);

    QObject::connect(&receiverThread, &QThread::finished,
                     receiver, &QObject::deleteLater);

    receiverThread.start();

    // ---------------- QML ENGINE ----------------
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", &controller);
    engine.rootContext()->setContextProperty("itemModel", &model);

    engine.loadFromModule("Client", "Main");

    if (engine.rootObjects().isEmpty()) {
        senderThread.quit();
        senderThread.wait();

        receiverThread.quit();
        receiverThread.wait();
        return -1;
    }

    int result = app.exec();

    senderThread.quit();
    senderThread.wait();

    receiverThread.quit();
    receiverThread.wait();

    return result;
}