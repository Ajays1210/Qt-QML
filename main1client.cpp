// main.cpp (client)
// Usage: task_client              (connects to 127.0.0.1)
//        task_client 192.168.1.5  (connects to that server address)
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString host = "127.0.0.1";
    if (argc > 1)
        host = argv[1];

    MainWindow window(host);
    window.show();

    return app.exec();
}
