#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Widgets application object (needed for any QWidget GUI).
    QApplication app(argc, argv);

    // Note: on Qt 5 you would also need qRegisterMetaType<...>() here for the
    // three response structs, so Qt can copy them between threads. Qt 6 does
    // this by itself, so nothing is needed.

    MainWindow window;   // builds the screen and starts the network thread
    window.show();

    return app.exec();   // the GUI event loop runs in the main thread
}