#include <QCoreApplication>
#include <QDebug>
#include "server.h"

int main(int argc, char *argv[])
{
    // Console app: no GUI needed on the server side.
    QCoreApplication app(argc, argv);

    qInfo() << "=== QT TASK TCP/UDP SERVER ===";

    Server server;
    server.start();   // starts the TCP thread and the UDP thread

    // When the app quits, stop both threads cleanly.
    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &server, &Server::stop);

    return app.exec();   // main thread just runs the event loop
}