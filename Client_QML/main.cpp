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
    // GUI application object (the server used QCoreApplication, no GUI).
    QGuiApplication app(argc, argv);

    // These two live in the MAIN (GUI) thread.
    Model model;                  // the list data that QML displays
    Controller controller(&model);   // QML buttons call this; it also updates the model

    // ---------------- TCP SEND THREAD ----------------
    // Thread 1: sends commands to the server on TCP 4001.
    QThread senderThread;
    auto *sender = new TcpSender;       // no parent, so it can be moved to another thread
    sender->moveToThread(&senderThread);   // its slots now run in senderThread

    // When the thread starts, connect to the server (runs inside senderThread).
    QObject::connect(&senderThread, &QThread::started,
                     sender, &TcpSender::connectToServer);

    // Controller (main thread) -> TcpSender (sender thread).
    // Queued = the bytes are copied and delivered safely across threads.
    QObject::connect(&controller, &Controller::sendBytes,
                     sender, &TcpSender::sendCommand,
                     Qt::QueuedConnection);

    // Delete the sender object when its thread finishes.
    QObject::connect(&senderThread, &QThread::finished,
                     sender, &QObject::deleteLater);

    senderThread.start();

    // ---------------- UDP RECEIVE THREAD ----------------
    // Thread 2: receives the server's responses on UDP 4002.
    QThread receiverThread;
    auto *receiver = new UdpReceiver;
    receiver->moveToThread(&receiverThread);

    // When the thread starts, bind the UDP port (runs inside receiverThread).
    QObject::connect(&receiverThread, &QThread::started,
                     receiver, &UdpReceiver::startListening);

    // UdpReceiver (receiver thread) -> Controller (main thread).
    // Each response type has its own signal and slot. The slot updates the list.
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
    // Loads the UI. These two lines make "controller" and "itemModel"
    // usable by name inside Main.qml.
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", &controller);
    engine.rootContext()->setContextProperty("itemModel", &model);

    // Loads Main.qml from the QML module named "Client".
    engine.loadFromModule("Client", "Main");

    // If the QML failed to load, stop both threads and exit with an error.
    if (engine.rootObjects().isEmpty()) {
        senderThread.quit();
        senderThread.wait();

        receiverThread.quit();
        receiverThread.wait();
        return -1;
    }

    // Main thread runs the GUI event loop until the window is closed.
    int result = app.exec();

    // Window closed: stop each thread's event loop and wait for it to finish.
    senderThread.quit();
    senderThread.wait();

    receiverThread.quit();
    receiverThread.wait();

    return result;
}